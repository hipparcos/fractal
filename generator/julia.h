#ifndef H_JULIA
#define H_JULIA

#include "../fractal.h"

struct color julia(
    int x, int y, int w, int h,
    double lx, double ly,
    double lxmin, double lxmax, double lymin, double lymax,
    int imax);

#endif
