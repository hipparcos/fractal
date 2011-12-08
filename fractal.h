#ifndef H_FRACTAL
#define H_FRACTAL

#include <SDL/SDL.h>

#include "color.h"
#include "frame.h"

struct fractal;

typedef struct color (*fractal_generator)(
    int x, int y, int w, int h,
    double lxmin, double lxmax, double lymin, double lymax,
    int imax);

struct fractal* fractal_create(int width, int height, int bpp, fractal_generator, int imax);
void fractal_destroy(struct fractal*);
struct frame* fractal_get_frame(struct fractal*);
void fractal_clear(struct fractal*);
void fractal_display(struct fractal*);
void fractal_update(struct fractal*);

void fractal_env_init(const char*);
void fractal_env_quit(void);
void fractal_env_set_caption(const char*);

#endif
