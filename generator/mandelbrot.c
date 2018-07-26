#include "mandelbrot.h"

int mandelbrot(double lx, double ly, int max_iter) {
    int iter = 0;
    double zrealpart = 0;
    double zimgpart = 0;
    double zmodule = 0;
    double tmprp = 0;

    double realpart = lx;
    double imgpart = ly;

    while ((zmodule < 4) && (iter < max_iter)) {
        tmprp = zrealpart;
        zrealpart = (zrealpart * zrealpart) - (zimgpart * zimgpart) + realpart;
        zimgpart = (2 * tmprp * zimgpart) + imgpart;
        zmodule = (zrealpart * zrealpart) + (zimgpart * zimgpart);
        iter++;
    }

    return iter;
}
