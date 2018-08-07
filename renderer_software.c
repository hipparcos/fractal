#include "renderer_software.h"

#include <limits.h>
#include <math.h>
#include <SDL2/SDL.h>

#include "panic.h"
#include "generator/julia.h"
#include "generator/julia_multiset.h"
#include "generator/mandelbrot.h"

#ifdef MT
#include <pthread.h>
#include <sys/sysinfo.h>
#include <time.h>
#endif

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
    bool done;
};

/* Workers */
#ifdef MT
static pthread_t* workers;
static size_t workerc;
static struct rdr_context* worker_ctx;
static pthread_mutex_t* worker_mutex_work;
static pthread_cond_t* worker_cond_work;
static pthread_mutex_t worker_mutex_done;
static pthread_cond_t worker_cond_done;
#endif

/** worker takes a rdr_context* and returns NULL. */
typedef void* (*worker)(void*);

/* Workers */
static void* rdr_sw_area_worker(void* arg);
static void* rdr_sw_line_worker(void* arg);

#ifdef MT
static void rdr_sw_threads_init(worker wk) {
    workerc = (size_t)get_nprocs();
    workers = calloc(workerc, sizeof(pthread_t));
    worker_mutex_work = calloc(workerc, sizeof(pthread_mutex_t));
    worker_cond_work = calloc(workerc, sizeof(pthread_cond_t));
    worker_ctx = calloc(workerc, sizeof(struct rdr_context));
    pthread_mutex_init(&worker_mutex_done, NULL);
    pthread_cond_init(&worker_cond_done, NULL);
    for (size_t w = 0; w < workerc; w++) {
        /* Worker argument */
        worker_ctx[w].buf = NULL;
        worker_ctx[w].workeri = w;
        worker_ctx[w].workerc = workerc;
        worker_ctx[w].done = false;
        /* Launch worker */
        pthread_mutex_init(&worker_mutex_work[w], NULL);
        pthread_cond_init(&worker_cond_work[w], NULL);
        if (0 != (pthread_create(&workers[w], NULL, wk, &worker_ctx[w]))) {
            panic("Error while launching thread.");
        }
    }
}
#endif

#ifdef MT
static void rdr_sw_threads_free(void) {
    for (size_t w = 0; w < workerc; w++) {
        pthread_cancel(workers[w]);
        pthread_mutex_destroy(&worker_mutex_work[w]);
        pthread_cond_destroy(&worker_cond_work[w]);
    }
    pthread_mutex_destroy(&worker_mutex_done);
    pthread_cond_destroy(&worker_cond_done);
    free(workers);
    free(worker_ctx);
}
#endif

void rdr_sw_init(SDL_Window* window) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    fractal.renderer = SDL_CreateRenderer(window, -1, 0);
    if (!fractal.renderer) {
        rdr_sw_free();
        panic("Error: SDL can't create a renderer.");
    }
    rdr_sw_resize(width, height);
#ifdef MT
    rdr_sw_threads_init(rdr_sw_area_worker);
#endif
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
#ifdef LT
    rdr_sw_threads_free();
#endif
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
#ifdef MT
    int w = ctx->workeri;
    while (true) {
        /* Wait for work order to be given. */
        pthread_mutex_lock(&worker_mutex_work[w]);
        pthread_cond_wait(&worker_cond_work[w], &worker_mutex_work[w]);
        ctx->done = false;
        pthread_mutex_unlock(&worker_mutex_work[w]);
#endif
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
#ifdef MT
        pthread_mutex_lock(&worker_mutex_done);
        ctx->done = true;
        pthread_cond_signal(&worker_cond_done);
        pthread_mutex_unlock(&worker_mutex_done);
    }
#endif
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
#ifdef MT
    int w = ctx->workeri;
    while (true) {
        /* Wait for work order to be given. */
        pthread_mutex_lock(&worker_mutex_work[w]);
        pthread_cond_wait(&worker_cond_work[w], &worker_mutex_work[w]);
        ctx->done = false;
        pthread_mutex_unlock(&worker_mutex_work[w]);
#endif
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
#ifdef MT
        pthread_mutex_lock(&worker_mutex_done);
        ctx->done = true;
        pthread_cond_signal(&worker_cond_done);
        pthread_mutex_unlock(&worker_mutex_done);
    }
#endif
    return NULL;
}

#ifdef MT
static void rdr_sw_update_mt(SDL_Surface* buf, struct fractal_info fi, double t) {
    /* Set constant for dynamic fractals. */
    if (fi.dynamic) {
        double tp = t / (2 * M_PI_2);
        double ct = cos(tp);
        double st = sin(tp);
        fi.jx *= ct;
        fi.jy *= st;
    }
    /* Update worker context. */
    for (size_t w = 0; w < workerc; w++) {
        pthread_mutex_lock(&worker_mutex_work[w]);
        worker_ctx[w].buf = buf;
        worker_ctx[w].fi = fi;
        worker_ctx[w].done = false;
        /* Signal execution to worker. */
        pthread_cond_signal(&worker_cond_work[w]);
        pthread_mutex_unlock(&worker_mutex_work[w]);
    }
    /* Timeout */
    struct timespec abstime;
    abstime.tv_sec  = 0;
    abstime.tv_nsec = 10000;
    /* Wait for all workers to finish. */
    while (true) {
        pthread_mutex_lock(&worker_mutex_done);
        pthread_cond_timedwait(&worker_cond_done, &worker_mutex_done, &abstime);
        bool done = true;
        for (size_t w = 0; w < workerc; w++) {
            done &= worker_ctx[w].done;
            /* Try to delock thread. */
            if (!worker_ctx[w].done) {
                pthread_mutex_lock(&worker_mutex_work[w]);
                pthread_cond_signal(&worker_cond_work[w]);
                pthread_mutex_unlock(&worker_mutex_work[w]);
            }
        }
        if (done) {
            pthread_mutex_unlock(&worker_mutex_done);
            break;
        }
        pthread_mutex_unlock(&worker_mutex_done);
    }
}
#endif

static void rdr_sw_update(SDL_Surface* buf, struct fractal_info fi, double t, worker wk) {
    /* Set constant for dynamic fractals. */
    if (fi.dynamic) {
        double tp = t / (2 * M_PI_2);
        double ct = cos(tp);
        double st = sin(tp);
        fi.jx *= ct;
        fi.jy *= st;
    }
    /* Update worker context. */
    struct rdr_context ctx= {0};
    ctx.buf = buf;
    ctx.fi = fi;
    ctx.workeri = 0;
    ctx.workerc = 1;
    /* Launch worker. */
    wk(&ctx);
}

void rdr_sw_render(struct fractal_info fi, double t, double dt) {
    (void)dt;
    /* Update main memory buffer. */
#ifdef MT
    rdr_sw_update_mt(fractal.buffer, fi, t);
#else
    rdr_sw_update(fractal.buffer, fi, t, rdr_sw_area_worker);
#endif
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
