#include "julia.h"

#ifndef JULIA_C
#define JULIA_C_real -0.8
#define JULIA_C_imag 0.156
#endif

int julia(double lx, double ly, int max_iter) {
    int iter = 0;
    double zrealpart = lx;
    double zimgpart = ly;
    double zmodule = 0;
    double tmprp = 0;

    while ((zmodule < 4) && (iter < max_iter)) {
        tmprp = zrealpart;
        zrealpart = (zrealpart * zrealpart) - (zimgpart * zimgpart) + JULIA_C_real;
        zimgpart = (2 * tmprp * zimgpart) + JULIA_C_imag;
        zmodule = (zrealpart * zrealpart) + (zimgpart * zimgpart);
        iter++;
    }

    return iter;
}
