#include "julia.h"

int julia(double ix, double iy, double cx, double cy, int max_iter) {
    int iter = 0;
    double zrealpart = ix;
    double zimgpart = iy;
    double zmodule = 0;
    double tmprp = 0;

    while ((zmodule < 4) && (iter < max_iter)) {
        tmprp = zrealpart;
        zrealpart = (zrealpart * zrealpart) - (zimgpart * zimgpart) + cx;
        zimgpart = (2 * tmprp * zimgpart) + cy;
        zmodule = (zrealpart * zrealpart) + (zimgpart * zimgpart);
        iter++;
    }

    return iter;
}
