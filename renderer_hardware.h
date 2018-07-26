#ifndef _H_RENDERER_HARDWARE_
#define _H_RENDERER_HARDWARE_

#include <stdbool.h>
#include <SDL2/SDL.h>

#include "types.h"

/* renderer interface */
void rdr_hw_init(SDL_Window* window, struct fractal_info fi);
void rdr_hw_free(void);
void rdr_hw_set_generator(enum generator gen);
void rdr_hw_set_center(double cx, double cy);
void rdr_hw_set_dpp(double dpp);
void rdr_hw_translate(double dx, double dy);
void rdr_hw_zoom(double factor);
void rdr_hw_resize(int width, int height);
void rdr_hw_render(unsigned long max_iter);

struct renderer hw_renderer = {
    .init          = rdr_hw_init,
    .free          = rdr_hw_free,
    .set_generator = rdr_hw_set_generator,
    .set_center    = rdr_hw_set_center,
    .set_dpp       = rdr_hw_set_dpp,
    .translate     = rdr_hw_translate,
    .zoom          = rdr_hw_zoom,
    .resize        = rdr_hw_resize,
    .render        = rdr_hw_render,
};

#endif
