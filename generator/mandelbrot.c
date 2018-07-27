#include "mandelbrot.h"

#include "julia.h"

int mandelbrot(double ix, double iy, double cx, double cy, int n, int max_iter) {
    (void)cx;
    (void)cy;
    (void)n;
    return julia(0.0, 0.0, ix, iy, 1, max_iter);
}
