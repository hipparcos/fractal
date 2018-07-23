#ifndef H_FRACTAL
#define H_FRACTAL

#include <stdint.h>

#include "frame.h"

struct win_info {
    char* title;
    int width;
    int height;
    int bpp;
};

typedef unsigned long (*fractal_generator)(double lx, double ly, unsigned long max_iter);

struct fractal_info {
    fractal_generator generator;
    struct frame default_frame;
    unsigned long max_iter;
};

struct fractal;

struct fractal* fractal_create(struct win_info, struct fractal_info);
void fractal_destroy(struct fractal*);
void fractal_clear(struct fractal*);
void fractal_update(struct fractal*, struct frame*);
const uint32_t* fractal_get_pixels(struct fractal* f);

void fractal_set_generator(struct fractal* f, fractal_generator gen);
void fractal_max_iter_incr(struct fractal*, unsigned long step);
void fractal_max_iter_decr(struct fractal*, unsigned long step);
void fractal_set_max_iter(struct fractal* f, unsigned long max_iter);

#endif
