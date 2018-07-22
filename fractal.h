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
unsigned long fractal_get_max_iter(struct fractal*);
void fractal_set_max_iter(struct fractal*, unsigned long max_iter);
int fractal_get_width(struct fractal*);
int fractal_get_height(struct fractal*);
struct frame fractal_get_default_frame(struct fractal*);
unsigned long fractal_get_default_max_iter(struct fractal*);

#endif
