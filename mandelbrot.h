#ifndef H_MANDELBROT
#define H_MANDELBROT

#include "fractal.h"

Uint32 mandelbrot(
    int x, int y, int w, int h,
    double lxmin, double lxmax, double lymin, double lymax
    );

#endif
