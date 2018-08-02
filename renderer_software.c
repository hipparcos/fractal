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

#define BPP 32
#ifndef M_PI
#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923
#endif

typedef int (*fractal_generator)(double ix, double iy, double cx, double cy, int n, int max_iter);

static struct {
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_Surface* buffer;
    fractal_generator generator;
    double cx, cy;
    double dpp;
} fractal = {
    .cx= .0, .cy= .0, .dpp= 0.01, // default values.
};

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
    fractal.buffer = SDL_CreateRGBSurface(0, width, height, BPP, 0, 0, 0, 0);
    if (!fractal.buffer) {
        rdr_sw_free();
        panic("Error: SDL can't create a surface.");
    }
}

struct rdr_context {
    struct fractal_info fi;
    fractal_generator gen;
    int first_line;
    int last_line;
};

static void* rdr_worker(void* arg) {
    struct rdr_context* ctx = (struct rdr_context*) arg;

    for(int y = ctx->first_line; y < ctx->last_line; y++) {
        for(int x = 0; x < fractal.buffer->w; x++) {
            // Calculate a pixel.
            int iter = ctx->gen(
                        ctx->fi.cx + ctx->fi.dpp * (x - fractal.buffer->w/2), // ix.
                        ctx->fi.cy + ctx->fi.dpp * (y - fractal.buffer->h/2), // iy.
                        ctx->fi.jx,
                        ctx->fi.jy,
                        ctx->fi.n,
                        ctx->fi.max_iter);

            if (iter == ctx->fi.max_iter) {
                iter = 0;
            }

            /* Color */
            double ratio = (double)(iter) / (double)(ctx->fi.max_iter);
            int color = floor((double)(0xff) * ratio);
            int red   = color;
            int green = color;
            int blue  = color;

            /* Set pixel color. */
            *((Uint32*)(fractal.buffer->pixels) + x + y * fractal.buffer->w)
                = SDL_MapRGB(fractal.buffer->format, red, green, blue);
        }
    }
    return NULL;
}

static void rdr_sw_update(struct fractal_info fi, fractal_generator gen, double t, double dt) {
    (void)dt;
    /* SDL_FillRect(fractal.buffer, NULL, SDL_MapRGB(fractal.buffer->format, 0, 0, 0)); */

    if (fi.dynamic) {
        double tp = t / (2 * M_PI_2);
        double ct = cos(tp);
        double st = sin(tp);
        fi.jx *= ct;
        fi.jy *= st;
    }

    size_t num_threads = (size_t)get_nprocs();
    pthread_t threads[num_threads];
    struct rdr_context ctx[num_threads];
    int step = fractal.buffer->h / num_threads;

    /* Launch threads. */
    for (size_t t = 0; t < num_threads; t++) {
        int last = (t+1) * step;
        if (last > fractal.buffer->h) last = fractal.buffer->h;
        ctx[t].fi= fi;
        ctx[t].gen= gen;
        ctx[t].first_line= t * step;
        ctx[t].last_line= last;
        if (0 != (pthread_create(&threads[t], NULL, rdr_worker, &ctx[t]))) {
            panic("Error while launching thread.");
        }
    }
    /* Waiting for all threads to finish. */
    for (size_t t = 0; t < num_threads; t++) {
        if (0 != (pthread_join(threads[t], NULL))) {
            panic("Error while joining thread.");
        }
    }

    Uint32* pixels = (Uint32*) fractal.buffer->pixels;
    SDL_UpdateTexture(fractal.texture, NULL, pixels, fractal.buffer->w * sizeof(Uint32));
}

void rdr_sw_render(struct fractal_info fi, double t, double dt) {
    rdr_sw_update(fi, rdr_sw_get_generator(fi.generator), t, dt);
    SDL_RenderClear(fractal.renderer);
    SDL_SetRenderDrawColor(fractal.renderer, 0, 0, 0, 255);
    SDL_RenderCopy(fractal.renderer, fractal.texture, NULL, NULL);
    SDL_RenderPresent(fractal.renderer);
}
