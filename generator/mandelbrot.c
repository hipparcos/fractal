#include "mandelbrot.h"

struct color mandelbrot(double lx, double ly, int imax) {
    /* Initialisation */
    int n = 0; /* iterator */
    int colorf = 255/imax; /* depth */
    double zrealpart = 0;
    double zimgpart = 0;
    double zmodule = 0;
    double tmprp = 0;

    /* Data */
    double realpart = lx;
    double imgpart = ly;

    /* Algorythm */
    while ((zmodule < 4) && (n < imax)) {
        tmprp = zrealpart;
        zrealpart = (zrealpart * zrealpart) - (zimgpart * zimgpart) + realpart;
        zimgpart = (2 * tmprp * zimgpart) + imgpart;
        zmodule = (zrealpart * zrealpart) + (zimgpart * zimgpart);
        n++;
    }

    /* Color */
    int red,green,blue;

    if (n < imax) {
        red = green = blue = n * colorf;
    } else {
        red = green = blue = 0;
    }

    struct color c = {red, green, blue};

    return c;
}
