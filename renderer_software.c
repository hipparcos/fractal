#include "renderer_software.h"

#include <limits.h>
#include <math.h>
#include <SDL2/SDL.h>

#include "panic.h"
#include "generator/julia.h"
#include "generator/mandelbrot.h"

#define BPP 32

typedef int (*fractal_generator)(double lx, double ly, int max_iter);

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
    case GEN_MANDELBROT:
        return mandelbrot;
    default:
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

static void rdr_sw_update(struct fractal_info fi, fractal_generator gen) {
    SDL_FillRect(fractal.buffer, NULL, SDL_MapRGB(fractal.buffer->format, 0, 0, 0));

    for(int y = 0; y < fractal.buffer->h; y++) {
        for(int x = 0; x < fractal.buffer->w; x++) {
            // Calculate a pixel.
            int iter = gen(
                    fi.cx + fi.dpp * (x - fractal.buffer->w/2), // lx.
                    fi.cy + fi.dpp * (y - fractal.buffer->h/2), // ly.
                    fi.max_iter);

            if (iter == fi.max_iter) {
                iter = 0;
            }

            /* Color */
            double ratio = (double)(iter) / (double)(fi.max_iter);
            int color = floor((double)(0xff) * ratio);
            int red   = color;
            int green = color;
            int blue  = color;

            /* Set pixel color. */
            *((Uint32*)(fractal.buffer->pixels) + x + y * fractal.buffer->w)
                = SDL_MapRGB(fractal.buffer->format, red, green, blue);
        }
    }

    Uint32* pixels = (Uint32*) fractal.buffer->pixels;
    SDL_UpdateTexture(fractal.texture, NULL, pixels, fractal.buffer->w * sizeof(Uint32));
}

void rdr_sw_render(struct fractal_info fi) {
    rdr_sw_update(fi, rdr_sw_get_generator(fi.generator));
    SDL_RenderClear(fractal.renderer);
    SDL_SetRenderDrawColor(fractal.renderer, 0, 0, 0, 255);
    SDL_RenderCopy(fractal.renderer, fractal.texture, NULL, NULL);
    SDL_RenderPresent(fractal.renderer);
}
