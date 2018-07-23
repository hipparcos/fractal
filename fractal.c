#include "fractal.h"

#include <limits.h>
#include <math.h>
#include <SDL2/SDL.h>

struct fractal {
    SDL_Surface* buffer;
    fractal_generator generator;
    unsigned long max_iter;
};

struct fractal* fractal_create(struct win_info wi, struct fractal_info fi) {
    struct fractal* f = malloc(sizeof(struct fractal));
    if (!f) {
        return NULL;
    }

    f->buffer = SDL_CreateRGBSurface(0, wi.width, wi.height, wi.bpp, 0, 0, 0, 0);
    if (!f->buffer) {
        free(f);
        return NULL;
    }
    f->generator = fi.generator;
    f->max_iter = fi.max_iter;

    return f;
}

void fractal_destroy(struct fractal* f) {
    if (!f) {
        return;
    }

    if (f->buffer) {
        SDL_FreeSurface(f->buffer);
    }
    free(f);
}

void fractal_update(struct fractal* f, struct frame* fm) {
    if (!f) {
        return;
    }

    SDL_FillRect(f->buffer, NULL, SDL_MapRGB(f->buffer->format, 0, 0, 0));

    for(int y = 0; y < f->buffer->h; y++) {
        for(int x = 0; x < f->buffer->w; x++) {
            // Calculate a pixel.
            unsigned long iter = f->generator(
                                     frame_globalx_to_localx(fm, x, f->buffer->w),  // lx
                                     frame_globaly_to_localy(fm, y, f->buffer->h),  // ly
                                     f->max_iter);

            if (iter == f->max_iter) {
                iter = 0;
            }

            /* Color */
            double ratio = (double)(iter) / (double)(f->max_iter);
            int color = floor((double)(0xff) * ratio);
            int red   = color;
            int green = color;
            int blue  = color;

            /* Set pixel color. */
            *((Uint32*)(f->buffer->pixels) + x + y * f->buffer->w)
                = SDL_MapRGB(f->buffer->format, red, green, blue);
        }
    }
}

const uint32_t* fractal_get_pixels(struct fractal* f) {
    if (!f) {
        return NULL;
    }

    return f->buffer->pixels;
}

void fractal_max_iter_incr(struct fractal* f, unsigned long step) {
    if (!f) {
        return;
    }

    if (f->max_iter > ULONG_MAX - step) {
        f->max_iter = ULONG_MAX;
    } else {
        f->max_iter += step;
    }
}

void fractal_max_iter_decr(struct fractal* f, unsigned long step) {
    if (!f) {
        return;
    }

    if (f->max_iter < step) {
        f->max_iter = 0;
    } else {
        f->max_iter -= step;
    }
}

void fractal_set_max_iter(struct fractal* f, unsigned long max_iter) {
    if (!f) {
        return;
    }

    f->max_iter = max_iter;
}
