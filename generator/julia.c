#include "julia.h"

int julia(double ix, double iy, double cx, double cy, int n, int max_iter) {
    (void)n;
    int iter = 0;
    double z_real = ix;
    double z_imag = iy;
    double t_real = 0;

    for (iter = 0; iter < max_iter; iter++) {
        t_real = z_real;
        z_real = (z_real * z_real) - (z_imag * z_imag) + cx;
        z_imag = (2 * t_real * z_imag) + cy;
        if (z_real * z_real + z_imag * z_imag > 4.0) {
            break;
        }
    }

    return iter;
}
