#ifndef _H_BENCHMARK_WORKER_
#define _H_BENCHMARK_WORKER_

#ifndef WIDTH
#define WIDTH 800
#endif

#ifndef HEIGHT
#define HEIGHT 600
#endif

#ifndef RUNS
#define RUNS 1000
#endif

#define _STRINGIFY(str) #str
#define STRINGIFY(str) _STRINGIFY(str)

#define benchmark_worker(wk, runs, width, height) \
    printf("Benchmarking %s\n", STRINGIFY(wk)); \
    printf("  definition: %dx%d\n", width, height); \
    printf("  runs: %d\n", runs); \
    _benchmark_worker(wk, runs, width, height);

void _benchmark_worker(worker wk, int runs, int width, int height) {
    /* Init */
    SDL_Surface* buffer;
    buffer = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    struct fractal_info fi = {
        .generator = GEN_MANDELBROT,
        .max_iter  = 50,
        .cx        = -0.7,
        .cy        = 0.0,
        .dpp       = 0.0035,
    };

    /* Benchmark */
    for (int i = 0; i < runs; i++) {
        rdr_sw_update(buffer, fi, 0.0, wk);
    }

    /* Cleanup */
    SDL_FreeSurface(buffer);
}

#endif
