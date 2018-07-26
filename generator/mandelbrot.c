#include "mandelbrot.h"

#include "julia.h"

int mandelbrot(double ix, double iy, double cx, double cy, int max_iter) {
    (void)cx;
    (void)cy;
    return julia(0.0, 0.0, ix, iy, max_iter);
}
