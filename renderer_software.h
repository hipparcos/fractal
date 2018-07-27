#ifndef H_FRACTAL
#define H_FRACTAL

#include <stdbool.h>
#include <SDL2/SDL.h>

#include "types.h"

/* renderer interface */
void rdr_sw_init(SDL_Window* window);
void rdr_sw_free(void);
void rdr_sw_resize(int width, int height);
void rdr_sw_render(struct fractal_info fi, double t, double dt);

struct renderer sw_renderer = {
    .init   = rdr_sw_init,
    .free   = rdr_sw_free,
    .resize = rdr_sw_resize,
    .render = rdr_sw_render,
};

#endif
