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

void fi_print(struct fractal_info* fi) {
    if (!fi) {
        return;
    }

    FILE* out = stdout;
    fprintf(out, "fractal_info {\n");
    fprintf(out, "  .generator= %d\n", fi->generator);
    fprintf(out, "  .dynamic=   %s\n", (fi->dynamic) ? "true" : "false");
    fprintf(out, "  .speed=     %lf\n", fi->speed);
    fprintf(out, "  .max_iter=  %d\n", fi->max_iter);
    fprintf(out, "  .cx=        %lf\n", fi->cx);
    fprintf(out, "  .cy=        %lf\n", fi->cy);
    fprintf(out, "  .dpp=       %lf\n", fi->dpp);
    fprintf(out, "  .jx=        %lf\n", fi->jx);
    fprintf(out, "  .jy=        %lf\n", fi->jy);
    fprintf(out, "  .n=         %d\n", fi->n);
    fprintf(out, "}\n");
}
