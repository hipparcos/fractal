#ifndef _H_TYPES_
#define _H_TYPES_

enum generator {
    GEN_MANDELBROT,
    GEN_JULIA,
};

struct fractal_info {
    enum generator generator;
    double cx;  // center of view X in local coord.
    double cy;  // center of viex Y in local coord.
    double dpp; // density per pixel.
};

struct renderer {
    void (*set_generator)(enum generator);
    void (*set_center)(double cx, double cy);
    void (*set_dpp)(double dpp);
    void (*translate)(double dx, double dy);
    void (*zoom)(double factor);
    void (*render)(unsigned long max_iter);
};

#endif
