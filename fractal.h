#ifndef H_FRACTAL
#define H_FRACTAL

#include "frame.h"

struct win_info {
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
void fractal_display(struct fractal*);
void fractal_update(struct fractal*, struct frame*);

void fractal_max_iter_incr(struct fractal*, unsigned long step);
void fractal_max_iter_decr(struct fractal*, unsigned long step);
void fractal_set_max_iter(struct fractal* f, unsigned long max_iter);

#endif
