#include "types.h"

#include <limits.h>

void fi_max_iter_incr(struct fractal_info* fi, int step) {
    if (fi->max_iter > INT_MAX - step) {
        fi->max_iter = INT_MAX;
    } else {
        fi->max_iter += step;
    }
}

void fi_max_iter_decr(struct fractal_info* fi, int step) {
    if (fi->max_iter < step) {
        fi->max_iter = 0;
    } else {
        fi->max_iter -= step;
    }
}

void fi_translate(struct fractal_info* fi, SDL_Window* window, double dx, double dy) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    fi->cx += dx * width * fi->dpp;
    fi->cy -= dy * height * fi->dpp;
}

void fi_zoom(struct fractal_info* fi, double factor) {
    if (factor < 0.001) {
        return;
    }
    fi->dpp *= 1/factor;
}
