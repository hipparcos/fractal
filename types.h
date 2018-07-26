#ifndef _H_TYPES_
#define _H_TYPES_

#include <SDL2/SDL.h>

enum generator {
    GEN_MANDELBROT,
    GEN_JULIA,
};

/** fractal_info gathers init informations about fractal for renderers. */
struct fractal_info {
    enum generator generator;
    /** max_iter is the maximum number of iteration for each pixel. */
    int max_iter;
    /** cx is the center of view x coord in local coord. */
    double cx;
    /** cy is the center of view y coord in local coord. */
    double cy;
    /** dpp is the density per pixel (width of each pixel in local coords). */
    double dpp;
};

/** renderer is the interface that all renderers must implement. */
struct renderer {
    /** init initializes the renderer. */
    void (*init)(SDL_Window* window);
    /** free cleans up renderer memory. */
    void (*free)(void);
    /** resize resizes the renderer. */
    void (*resize)(int width, int height);
    /** render renders the fractal to the screen. */
    void (*render)(struct fractal_info fi);
};

#endif
