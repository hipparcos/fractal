#ifndef H_FRACTAL
#define H_FRACTAL

#include "color.h"
#include "frame.h"

struct win_info {
    int width;
    int height;
    int bpp;
};

typedef struct color (*fractal_generator)(double lx, double ly, int imax);

struct fractal_info {
    fractal_generator generator;
    struct frame default_frame;
    int resolution;
};

struct fractal;

struct fractal* fractal_create(struct win_info, struct fractal_info);
void fractal_destroy(struct fractal*);
void fractal_clear(struct fractal*);
void fractal_display(struct fractal*);
void fractal_update(struct fractal*, struct frame*);
int fractal_get_imax(struct fractal*);
void fractal_set_imax(struct fractal*, int imax);
int fractal_get_width(struct fractal*);
int fractal_get_height(struct fractal*);
struct frame fractal_get_default_frame(struct fractal*);
int fractal_get_default_resolution(struct fractal*);

#endif
