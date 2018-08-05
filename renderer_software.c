#include "renderer_software.h"

#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "panic.h"
#include "generator/julia.h"
#include "generator/julia_multiset.h"
#include "generator/mandelbrot.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923
#endif

typedef int (*fractal_generator)(double ix, double iy, double cx, double cy, int n, int max_iter);

static struct {
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_Surface* buffer;
} fractal;

/* Workers arguments */
struct rdr_context {
    SDL_Surface* buf;
    struct fractal_info fi;
    int workeri; // worker index.
    int workerc; // worker count.
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

/* Workers */
static pthread_t* workers;
static size_t workerc;
static struct rdr_context* worker_ctx;

/** worker takes a rdr_context* and returns NULL. */
typedef void* (*worker)(void*);

/* Workers */
static void* rdr_sw_area_worker(void* arg);
static void* rdr_sw_line_worker(void* arg);

static void rdr_sw_threads_init(worker wk) {
    workerc = (size_t)get_nprocs();
    workers = calloc(workerc, sizeof(pthread_t));
    worker_ctx = calloc(workerc, sizeof(struct rdr_context));
    for (size_t w = 0; w < workerc; w++) {
        /* Worker argument */
        worker_ctx[w].buf = NULL;
        worker_ctx[w].workeri = w;
        worker_ctx[w].workerc = workerc;
        pthread_mutex_init(&worker_ctx[w].mutex, NULL);
        pthread_cond_init(&worker_ctx[w].cond, NULL);
        /* Launch worker */
        if (0 != (pthread_create(&workers[w], NULL, wk, &worker_ctx[w]))) {
            panic("Error while launching thread.");
        }
    }
}

static void rdr_sw_threads_free(void) {
    for (size_t w = 0; w < workerc; w++) {
        pthread_cancel(workers[w]);
        pthread_mutex_destroy(&worker_ctx[w].mutex);
        pthread_cond_destroy(&worker_ctx[w].cond);
    }
    free(workers);
    free(worker_ctx);
}

void rdr_sw_init(SDL_Window* window) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    fractal.renderer = SDL_CreateRenderer(window, -1, 0);
    if (!fractal.renderer) {
        rdr_sw_free();
        panic("Error: SDL can't create a renderer.");
    }
    rdr_sw_resize(width, height);
    rdr_sw_threads_init(rdr_sw_area_worker);
}

void rdr_sw_free(void) {
    if (fractal.renderer) {
        SDL_DestroyRenderer(fractal.renderer);
    }
    if (fractal.texture) {
        SDL_DestroyTexture(fractal.texture);
    }
    if (fractal.buffer) {
        SDL_FreeSurface(fractal.buffer);
    }
    rdr_sw_threads_free();
}

/* renderer interface */
static fractal_generator rdr_sw_get_generator(enum generator gen) {
    switch (gen) {
    case GEN_JULIA:
        return julia;
        break;
    case GEN_JULIA_MULTISET:
        return julia_multiset;
        break;
    default:
    case GEN_MANDELBROT:
        return mandelbrot;
        break;
    }
    return NULL;
}

void rdr_sw_resize(int width, int height) {
    /* New texture. */
    if (fractal.texture) {
        SDL_DestroyTexture(fractal.texture);
    }
    fractal.texture = SDL_CreateTexture(fractal.renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
           width, height);
    if (!fractal.texture) {
        rdr_sw_free();
        panic("Error: SDL can't create a texture.");
    }
    /* New surface. */
    if (fractal.buffer) {
        SDL_FreeSurface(fractal.buffer);
    }
    fractal.buffer = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    if (!fractal.buffer) {
        rdr_sw_free();
        panic("Error: SDL can't create a surface.");
    }
}


/** rdr_sw_line_worker renders a contiguous set of lines to buffer. */
static void* rdr_sw_line_worker(void* arg) {
    struct rdr_context* ctx = (struct rdr_context*) arg;
    int ret = 0;
    while (true) {
        /* Wait for work order to be given. */
        if (0 != (ret = pthread_mutex_lock(&ctx->mutex))) {
            panicf("Can't lock work mutex in worker. ret: %d", ret);
        }
        if (0 != (ret = pthread_cond_wait(&ctx->cond, &ctx->mutex))) {
            panicf("Can't wait on work cond in worker. ret: %d", ret);
        }
        /* Proxy variables. */
        int width  = ctx->buf->w;
        int height = ctx->buf->h;
        struct fractal_info fi = ctx->fi;
        fractal_generator gen = rdr_sw_get_generator(ctx->fi.generator);
        /* Worker specific. */
        int start_line = (ctx->workeri * height) / ctx->workerc;
        int lines_per_wk = height / ctx->workerc;
        /* Painting variables. */
        uint32_t* pixels = (uint32_t*)ctx->buf->pixels + start_line * width;
        SDL_PixelFormat* format = ctx->buf->format;

        /* Calculate iteration per pixel. */
        for(int y = start_line; y < start_line + lines_per_wk; y++) {
            for(int x = 0; x < width; x++) {
                // Calculate a pixel.
                int iter = gen(
                            fi.cx + fi.dpp * (x - width/2), // ix.
                            fi.cy + fi.dpp * (y - height/2), // iy.
                            fi.jx,
                            fi.jy,
                            fi.n,
                            fi.max_iter);

                if (iter == fi.max_iter) {
                    iter = 0;
                }

                double ratio = (double)(iter) / fi.max_iter;
                uint8_t color = (uint8_t)(ratio * 0xff);
                *(pixels++) = SDL_MapRGB(format, color, color, color);
            }
        }
        if (0 != (ret = pthread_mutex_unlock(&ctx->mutex))) {
            panicf("Can't unlock work mutex in worker. ret: %d", ret);
        }
    }
    return NULL;
}

/** rdr_sw_area_worker renders rectangles to buffer.
 ** Better load distribution than rdr_sw_line_worker.
 ** Example: 4 workers, the second one renders
 **     .x..
 **     ..x.
 **     ...x
 **     x...
 */
static void* rdr_sw_area_worker(void* arg) {
    struct rdr_context* ctx = (struct rdr_context*) arg;
    int ret = 0;
    while (true) {
        /* Wait for work order to be given. */
        if (0 != (ret = pthread_mutex_lock(&ctx->mutex))) {
            panicf("Can't lock work mutex in worker. ret: %d", ret);
        }
        if (0 != (ret = pthread_cond_wait(&ctx->cond, &ctx->mutex))) {
            panicf("Can't wait on work cond in worker. ret: %d", ret);
        }
        /* Proxy variables. */
        int width  = ctx->buf->w;
        int height = ctx->buf->h;
        struct fractal_info fi = ctx->fi;
        fractal_generator gen = rdr_sw_get_generator(ctx->fi.generator);
        /* Worker specific. */
        int recw = width / ctx->workerc;
        int rech = height / ctx->workerc;
        /* Painting variables. */
        uint32_t* pixels = ctx->buf->pixels;
        SDL_PixelFormat* format = ctx->buf->format;

        /* Calculate iteration per pixel. */
        int recoffset = ctx->workeri;
        int maxoffset = ctx->workerc - 1;
        for (int reci = 0; reci < ctx->workerc; reci++) {
            int xi = recoffset * recw;
            int yi = reci * rech;
            int xm = (xi + recw < width) ? xi + recw : width;
            if (recoffset == maxoffset) xm = width;
            int ym = (yi + rech < height) ? yi + rech : height;
            if (reci == maxoffset) ym = height;
            for (int y = yi; y < ym; y++) {
                for (int x = xi; x < xm; x++) {
                    // Calculate a pixel.
                    int iter = gen(
                                fi.cx + fi.dpp * (x - width/2), // ix.
                                fi.cy + fi.dpp * (y - height/2), // iy.
                                fi.jx,
                                fi.jy,
                                fi.n,
                                fi.max_iter);

                    if (iter == fi.max_iter) {
                        iter = 0;
                    }

                    double ratio = (double)(iter) / fi.max_iter;
                    uint8_t color = (uint8_t)(ratio * 0xff);
                    *(pixels + x + y * width) = SDL_MapRGB(format, color, color, color);
                }
            }
            recoffset = (recoffset + 1) % ctx->workerc;
        }
        if (0 != (ret = pthread_mutex_unlock(&ctx->mutex))) {
            panicf("Can't unlock work mutex in worker. ret: %d", ret);
        }
    }
    return NULL;
}

static void rdr_sw_update(SDL_Surface* buf, struct fractal_info fi, double t) {
    /* Set constant for dynamic fractals. */
    if (fi.dynamic) {
        double tp = t / (2 * M_PI_2);
        double ct = cos(tp);
        double st = sin(tp);
        fi.jx *= ct;
        fi.jy *= st;
    }
    int ret = 0;
    /* Dispatch work to workers. */
    for (size_t w = 0; w < workerc; w++) {
        /* Update worker context. */
        if (0 != (ret = pthread_mutex_lock(&worker_ctx[w].mutex))) {
            panicf("Can't lock work mutex in update. ret: %d", ret);
        }
        worker_ctx[w].buf = buf;
        worker_ctx[w].fi = fi;
        if (0 != (ret = pthread_mutex_unlock(&worker_ctx[w].mutex))) {
            panicf("Can't unlock work mutex in update. ret: %d", ret);
        }
        /* Signal execution to worker. */
        pthread_cond_signal(&worker_ctx[w].cond);
    }
    struct timespec req;
    req.tv_sec = 0;
    req.tv_nsec = 1000;
    struct timespec rem;
    nanosleep(&req, &rem);
    /* Wait for all workers to finish. */
    for (size_t w = 0; w < workerc; w++) {
        if (0 != (ret = pthread_mutex_lock(&worker_ctx[w].mutex))) {
            panicf("Can't lock finish mutex in update. ret: %d", ret);
        }
        /* Worker w has finished. */
        if (0 != (ret = pthread_mutex_unlock(&worker_ctx[w].mutex))) {
            panicf("Can't unlock finish mutex in update. ret: %d", ret);
        }
    }
}

void rdr_sw_render(struct fractal_info fi, double t, double dt) {
    (void)dt;
    /* Update main memory buffer. */
    rdr_sw_update(fractal.buffer, fi, t);
    /* Update GPU memory texture. */
    uint32_t* pixels; int pitch;
    SDL_LockTexture(fractal.texture, NULL, (void**)&pixels, &pitch);
    memcpy(pixels, fractal.buffer->pixels, fractal.buffer->h * pitch);
    SDL_UnlockTexture(fractal.texture);
    /* Render. */
    SDL_RenderClear(fractal.renderer);
    SDL_SetRenderDrawColor(fractal.renderer, 0, 0, 0, 255);
    SDL_RenderCopy(fractal.renderer, fractal.texture, NULL, NULL);
    SDL_RenderPresent(fractal.renderer);
}
