#include "julia.h"

#include <complex.h>

#ifndef JULIA_C
#define JULIA_C -0.8+0.156*I
//#define JULIA_C -0.4+0.6*I
//#define JULIA_C -0.70176-0.3842*I
//#define JULIA_C 0.285+0.01*I
#endif

struct color julia(double lx, double ly, int imax) {
    /* Initialisation */
    int n = 0; /* iterator */
    int colorf = 255/imax; /* depth */
    double zrealpart = lx;
    double zimgpart = ly;
    double zmodule = 0;
    double tmprp = 0;

    /* Algorythm */
    while ((zmodule < 4) && (n < imax)) {
        tmprp = zrealpart;
        zrealpart = (zrealpart * zrealpart) - (zimgpart * zimgpart) + creal(JULIA_C);
        zimgpart = (2 * tmprp * zimgpart) + cimag(JULIA_C);
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
