#include "fractal.h"

#include <math.h>
#include <SDL/SDL.h>

struct fractal {
    SDL_Surface* screen;
    SDL_Surface* buffer;
    fractal_generator generator;
    unsigned long max_iter;
    unsigned long default_max_iter;
    struct frame default_frame;
};

struct fractal* fractal_create(struct win_info wi, struct fractal_info fi) {
    struct fractal* f = malloc(sizeof(struct fractal));
    if (!f) {
        return NULL;
    }

    f->screen = SDL_SetVideoMode(wi.width, wi.height, wi.bpp, SDL_HWSURFACE);
    f->buffer = SDL_CreateRGBSurface(SDL_HWSURFACE,
                                     wi.width, wi.height, wi.bpp, 0, 0, 0, 0);
    f->generator = fi.generator;
    f->max_iter = fi.max_iter;
    f->default_frame = fi.default_frame;
    f->default_max_iter = fi.max_iter;

    fractal_clear(f);

    return f;
}

void fractal_destroy(struct fractal* f) {
    if (!f) {
        return;
    }
    if (f->buffer) {
        SDL_FreeSurface(f->buffer);
    }
    if (f->screen) {
        SDL_FreeSurface(f->screen);
    }
    free(f);
}

void fractal_clear(struct fractal* f) {
    if (!f) {
        return;
    }

    SDL_FillRect(f->screen, NULL, SDL_MapRGB(f->screen->format, 0, 0, 0));
}

void fractal_display(struct fractal* f) {
    if (!f) {
        return;
    }

    SDL_Flip(f->screen);
}

void fractal_update(struct fractal* f, struct frame* fm) {
    if (!f) {
        return;
    }

    SDL_FillRect(f->buffer, NULL, SDL_MapRGB(f->buffer->format, 0, 0, 0));

    for(int y = 0; y < f->screen->h; y++) {
        for(int x = 0; x < f->screen->w; x++) {
            // Calculate a pixel.
            unsigned long iter = f->generator(
                                     frame_globalx_to_localx(fm, x, f->screen->w),  // lx
                                     frame_globaly_to_localy(fm, y, f->screen->h),  // ly
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
            *((Uint32*)(f->buffer->pixels) + x + y * f->screen->w)
                = SDL_MapRGB(f->buffer->format, red, green, blue);
        }
    }

    SDL_Rect rect = {0};
    SDL_BlitSurface(f->buffer, NULL, f->screen, &rect);
}

unsigned long fractal_get_max_iter(struct fractal* f) {
    return f->max_iter;
}
void fractal_set_max_iter(struct fractal* f, unsigned long max_iter) {
    f->max_iter = max_iter;
}

int fractal_get_width(struct fractal* f) {
    return f->screen->w;
}

int fractal_get_height(struct fractal* f) {
    return f->screen->h;
}

struct frame fractal_get_default_frame(struct fractal* f) {
    return f->default_frame;
}

unsigned long fractal_get_default_max_iter(struct fractal* f) {
    return f->default_max_iter;
}
