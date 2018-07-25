#include "renderer_software.h"

#include <limits.h>
#include <math.h>
#include <SDL2/SDL.h>

#include "frame.h"
#include "panic.h"
#include "generator/julia.h"
#include "generator/mandelbrot.h"

#define BPP 32

typedef unsigned long (*fractal_generator)(double lx, double ly, unsigned long max_iter);

static struct {
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_Surface* buffer;
    fractal_generator generator;
    struct frame frame;
    double cx, cy;
    double dpp;
} fractal = {
    .cx= .0, .cy= .0, .dpp= 0.01, // default values.
};

void rdr_sw_init(SDL_Window* window, struct fractal_info fi) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    fractal.renderer = SDL_CreateRenderer(window, -1, 0);
    if (!fractal.renderer) {
        rdr_sw_free();
        panic("Error: SDL can't create a renderer.");
    }
    rdr_sw_resize(width, height);
    rdr_sw_set_generator(fi.generator);
    rdr_sw_set_center(fi.cx, fi.cy);
    rdr_sw_set_dpp(fi.dpp);
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
void rdr_sw_set_generator(enum generator gen) {
    switch (gen) {
    case GEN_JULIA:
        fractal.generator = julia;
        break;
    case GEN_MANDELBROT:
        fractal.generator = mandelbrot;
    default:
        break;
    }
}

static void rdr_sw_set_frame() {
    int width = fractal.buffer->w;
    int height = fractal.buffer->h;
    fractal.frame.ratio = (double)(width) / height;
    // Get boundaries from center & dpp.
    double xmin, xmax, ymin;
    xmin = fractal.cx - fractal.dpp * width/2;
    xmax = fractal.cx + fractal.dpp * width/2;
    ymin = fractal.cy - fractal.dpp * height/2;
    // Init frame & set ymax.
    frame_set(&fractal.frame, xmin, xmax, ymin);
}

void rdr_sw_set_center(double cx, double cy) {
    fractal.cx = cx;
    fractal.cy = cy;
    rdr_sw_set_frame();
}

void rdr_sw_set_dpp(double dpp) {
    fractal.dpp = dpp;
    rdr_sw_set_frame();
}

void rdr_sw_translate(double dx, double dy) {
    double lx = dx * frame_width(&fractal.frame);
    double ly = dy * frame_height(&fractal.frame);
    frame_translate(&fractal.frame, lx, ly);
    fractal.cx = (fractal.frame.xmax + fractal.frame.xmin) / 2;
    fractal.cy = (fractal.frame.ymax + fractal.frame.ymin) / 2;
}

void rdr_sw_zoom(double factor) {
    frame_zoom(&fractal.frame, factor);
    fractal.dpp = (fractal.frame.xmax - fractal.frame.xmin) / fractal.buffer->w;
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
    rdr_sw_set_frame();
}

static void rdr_sw_update(unsigned long max_iter) {
    SDL_FillRect(fractal.buffer, NULL, SDL_MapRGB(fractal.buffer->format, 0, 0, 0));

    for(int y = 0; y < fractal.buffer->h; y++) {
        for(int x = 0; x < fractal.buffer->w; x++) {
            // Calculate a pixel.
            unsigned long iter = fractal.generator(
                    frame_globalx_to_localx(&fractal.frame, x, fractal.buffer->w), // lx
                    frame_globaly_to_localy(&fractal.frame, y, fractal.buffer->h), // ly
                    max_iter);

            if (iter == max_iter) {
                iter = 0;
            }

            /* Color */
            double ratio = (double)(iter) / (double)(max_iter);
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

void rdr_sw_render(unsigned long max_iter) {
    rdr_sw_update(max_iter);
    SDL_RenderClear(fractal.renderer);
    SDL_SetRenderDrawColor(fractal.renderer, 0, 0, 0, 255);
    SDL_RenderCopy(fractal.renderer, fractal.texture, NULL, NULL);
    SDL_RenderPresent(fractal.renderer);
}
