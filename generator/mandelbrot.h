#ifndef H_MANDELBROT
#define H_MANDELBROT

#include "../fractal.h"

struct color mandelbrot(
    int x, int y, int w, int h,
    double lx, double ly,
    double lxmin, double lxmax, double lymin, double lymax,
    int imax);

#endif
