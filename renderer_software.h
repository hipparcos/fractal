#ifndef H_FRACTAL
#define H_FRACTAL

#include <stdbool.h>
#include <SDL2/SDL.h>

#include "types.h"

/* renderer interface */
void rdr_sw_init(SDL_Window* window, struct fractal_info fi);
void rdr_sw_free(void);
void rdr_sw_set_generator(enum generator gen);
void rdr_sw_set_center(double cx, double cy);
void rdr_sw_set_dpp(double dpp);
void rdr_sw_translate(double dx, double dy);
void rdr_sw_zoom(double factor);
void rdr_sw_resize(int width, int height);
void rdr_sw_render(unsigned long max_iter);

#endif
