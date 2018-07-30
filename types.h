#ifndef _H_TYPES_
#define _H_TYPES_

#include <stdbool.h>
#include <SDL2/SDL.h>

enum generator {
    GEN_MANDELBROT,
    GEN_JULIA,
    GEN_JULIA_MULTISET,
};

/** fractal_info gathers init informations about fractal for renderers. */
struct fractal_info {
    enum generator generator;
    /** dynamic tells if rendering should depend on time. */
    bool dynamic;
    /** speed controls dynamic fractals rendering speed (time multiplier). */
    double speed;
    /** max_iter is the maximum number of iteration for each pixel. */
    int max_iter;
    /** cx is the center of view x coord in local coord. */
    double cx;
    /** cy is the center of view y coord in local coord. */
    double cy;
    /** dpp is the density per pixel (width of each pixel in local coords). */
    double dpp;
    /** jx,jy are julia set init value. */
    double jx, jy;
    /** n is the power value in julia_multiset. */
    int n;
};

void fi_max_iter_incr(struct fractal_info* fi, int step);
void fi_max_iter_decr(struct fractal_info* fi, int step);
void fi_translate(struct fractal_info* fi, SDL_Window* window, double dx, double dy);
void fi_zoom(struct fractal_info* fi, double factor);
void fi_print(struct fractal_info* fi);

/** renderer is the interface that all renderers must implement. */
struct renderer {
    /** init initializes the renderer. */
    void (*init)(SDL_Window* window);
    /** free cleans up renderer memory. */
    void (*free)(void);
    /** resize resizes the renderer. */
    void (*resize)(int width, int height);
    /** render renders the fractal to the screen. */
    void (*render)(struct fractal_info fi, double t, double dt);
};

#endif
