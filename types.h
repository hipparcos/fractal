#ifndef _H_TYPES_
#define _H_TYPES_

struct win_info {
    char* title;
    int width;
    int height;
};

enum generator {
    GEN_MANDELBROT,
    GEN_JULIA,
};

struct fractal_info {
    enum generator generator;
    double xmin;
    double xmax;
    double ymin;
};

struct renderer {
    void (*set_generator)(enum generator);
    void (*set_frame)(double xmin, double xmax, double ymin);
    void (*translate)(double dx, double dy);
    void (*zoom)(double factor);
    void (*render)(unsigned long max_iter);
};

#endif
