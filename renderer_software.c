#include "renderer_software.h"

#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <sys/sysinfo.h>
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

void rdr_sw_init(SDL_Window* window) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    fractal.renderer = SDL_CreateRenderer(window, -1, 0);
    if (!fractal.renderer) {
        rdr_sw_free();
        panic("Error: SDL can't create a renderer.");
    }
    rdr_sw_resize(width, height);
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

struct rdr_context {
    SDL_Surface* buf;
    struct fractal_info fi;
    int workeri; // worker index.
    int workerc; // worker count.
};

/** worker takes a rdr_context* and returns NULL. */
typedef void* (*worker)(void*);

static void* rdr_sw_line_worker(void* arg) {
    struct rdr_context* ctx = (struct rdr_context*) arg;

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
    return NULL;
}

static void rdr_sw_update(SDL_Surface* buf, struct fractal_info fi, double t, worker wk) {
    /* Set constant for dynamic fractals. */
    if (fi.dynamic) {
        double tp = t / (2 * M_PI_2);
        double ct = cos(tp);
        double st = sin(tp);
        fi.jx *= ct;
        fi.jy *= st;
    }
    /* Thread pool. */
    size_t workerc = (size_t)get_nprocs();
    pthread_t threads[workerc];
    struct rdr_context ctx[workerc];
    /* Launch threads. */
    for (size_t t = 0; t < workerc; t++) {
        /* Create context for the thread t. */
        ctx[t].buf = buf;
        ctx[t].fi = fi;
        ctx[t].workeri = t;
        ctx[t].workerc = workerc;
        /* Launch worker execution. */
        if (0 != (pthread_create(&threads[t], NULL, wk, &ctx[t]))) {
            panic("Error while launching thread.");
        }
    }
    /* Waiting for all threads to finish. */
    for (size_t t = 0; t < workerc; t++) {
        if (0 != (pthread_join(threads[t], NULL))) {
            panic("Error while joining thread.");
        }
    }
}

void rdr_sw_render(struct fractal_info fi, double t, double dt) {
    (void)dt;
    /* Update main memory buffer. */
    rdr_sw_update(fractal.buffer, fi, t, rdr_sw_line_worker);
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
