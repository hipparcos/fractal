#include "julia_multiset.h"

#include <math.h>

int julia_multiset(double ix, double iy, double cx, double cy, int n, int max_iter) {
    int iter = 0;
    double z_real = ix;
    double z_imag = iy;

    for (iter = 0; iter < max_iter; iter++) {
        double x2 = z_real * z_real;
        double y2 = z_imag * z_imag;
        double at = atan2(z_imag, z_real);
        double cs = cos(n * at);
        double ss = sin(n * at);
        double p2 = pow(x2 + y2, n/2);
        double x = p2 * cs + cx;
        double y = p2 * ss + cy;
        if (x * x + y * y > 4.0) {
            break;
        }
        z_real = x;
        z_imag = y;
    }

    return iter;
}
