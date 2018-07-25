#ifndef _H_TYPES_
#define _H_TYPES_

enum generator {
    GEN_MANDELBROT,
    GEN_JULIA,
};

/** fractal_info gathers init informations about fractal for renderers. */
struct fractal_info {
    enum generator generator;
    /** cx is the center of view x coord in local coord. */
    double cx;
    /** cy is the center of view y coord in local coord. */
    double cy;
    /** dpp is the density per pixel (width of each pixel in local coords). */
    double dpp;
};

/** renderer is the interface that all renderers must implement. */
struct renderer {
    /** set_generator tells the renderer which fractal to draw. */
    void (*set_generator)(enum generator);
    /** set_center centers the view on cx,cy (in local coordinates). */
    void (*set_center)(double cx, double cy);
    /** set_dpp sets the width of each pixel in local coordinates. */
    void (*set_dpp)(double dpp);
    /** translate translates the view by dx * width & dy * height. */
    void (*translate)(double dx, double dy);
    /** zoom zooms on view. factor > 1 means zooming in, factor < 1 means zooming out. */
    void (*zoom)(double factor);
    /** resize resizes the view. width,height are in pixels. */
    void (*resize)(int width, int height);
    /** render renders the fractal to the screen. max_iter is the limit of iterations done per pixel. */
    void (*render)(unsigned long max_iter);
};

#endif
