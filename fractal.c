#include "fractal.h"

#include <SDL/SDL.h>

struct fractal {
    SDL_Surface* screen;
    SDL_Surface* buffer;
    fractal_generator generator;
    int imax;
    struct frame default_frame;
    int default_resolution;
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
    f->imax = fi.resolution;
    f->default_frame = fi.default_frame;
    f->default_resolution = fi.resolution;

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

static inline Uint32 color_to_sdl(struct color c, SDL_PixelFormat* fmt) {
    return SDL_MapRGB(fmt, c.r, c.g, c.b);
}
static Uint32 color_to_sdl(struct color c, SDL_PixelFormat* fmt);

void fractal_update(struct fractal* f, struct frame* fm) {
    if (!f) {
        return;
    }

    SDL_FillRect(f->buffer, NULL, SDL_MapRGB(f->buffer->format, 0, 0, 0));

    for(int y = 0; y < f->screen->h; y++) {
        for(int x = 0; x < f->screen->w; x++) {
            // Render a pixel.
            struct color c = f->generator(
                                 fm->xmin + x * (frame_width(fm) / f->screen->w),  // lx
                                 fm->ymin + y * (frame_height(fm) / f->screen->h), // ly
                                 f->imax);
            // Set pixel color.
            *((Uint32*)(f->buffer->pixels) + x + y * f->screen->w)
                = color_to_sdl(c, f->buffer->format);
        }
    }

    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    SDL_BlitSurface(f->buffer, NULL, f->screen, &rect);
}

int fractal_get_imax(struct fractal* f) {
    return f->imax;
}
void fractal_set_imax(struct fractal* f, int imax) {
    if(imax > 0) {
        f->imax = imax;
    }
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

int fractal_get_default_resolution(struct fractal* f) {
    return f->default_resolution;
}
