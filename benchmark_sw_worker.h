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

#define BENCHMARK_IMPL
#include "benchmark.h"

#define benchmark_worker(wk, runs, width, height) \
    benchmark_display_banner(STRINGIFY(wk), runs, "definition "STRINGIFY(width)"x"STRINGIFY(HEIGHT)); \
    long long startt = benchmark_get_time_ns(); \
    _benchmark_worker(wk, runs, width, height); \
    long long endtt = benchmark_get_time_ns(); \
    benchmark_display_results(startt, endtt, runs);

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

#ifdef MT
    rdr_sw_threads_init(wk);
#endif

    /* Benchmark */
    for (int i = 0; i < runs; i++) {
#ifdef MT
        rdr_sw_update_mt(buffer, fi, 0.0);
#else
        rdr_sw_update(buffer, fi, 0.0, wk);
#endif
    }

#ifdef MT
    rdr_sw_threads_free();
#endif

    /* Cleanup */
    SDL_FreeSurface(buffer);
}

#endif
