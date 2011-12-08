#ifndef H_FRACTAL
#define H_FRACTAL

#include <SDL/SDL.h>

struct fractal;

typedef Uint32 (*fractal_generator)(
    int x, int y, int w, int h,
    double lxmin, double lxmax, double lymin, double lymax
    );

struct fractal* fractal_create(int width, int height, int bpp, fractal_generator);
void fractal_destroy(struct fractal*);
void fractal_clear(struct fractal*);
void fractal_display(struct fractal*);
void fractal_update(struct fractal*);

#endif
