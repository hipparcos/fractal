#include "renderer_software.c"

#ifndef WIDTH
#define WIDTH 800
#endif

#ifndef HEIGHT
#define HEIGHT 600
#endif

#ifndef TIMES
#define TIMES 1000
#endif

#ifndef WORKER
#define WORKER rdr_sw_line_worker
#endif

int main(void)
{
    /* Init */
    SDL_Surface* buffer;
    buffer = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, 0, 0, 0, 0);
    struct fractal_info fi = {
        .generator = GEN_MANDELBROT,
        .max_iter  = 50,
        .cx        = -0.7,
        .cy        = 0.0,
        .dpp       = 0.0035,
    };

    /* Benchmark */
    for (int i = 0; i < TIMES; i++) {
        rdr_sw_update(buffer, fi, 0.0, WORKER);
    }

    /* Cleanup */
    SDL_FreeSurface(buffer);

    return EXIT_SUCCESS;
}
