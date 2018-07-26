#ifndef _H_RENDERER_HARDWARE_
#define _H_RENDERER_HARDWARE_

#include <stdbool.h>
#include <SDL2/SDL.h>

#include "types.h"

/* renderer interface */
void rdr_hw_init(SDL_Window* window);
void rdr_hw_free(void);
void rdr_hw_resize(int width, int height);
void rdr_hw_render(struct fractal_info fi);

struct renderer hw_renderer = {
    .init   = rdr_hw_init,
    .free   = rdr_hw_free,
    .resize = rdr_hw_resize,
    .render = rdr_hw_render,
};

#endif
