#ifndef _H_BENCHMARK_
#define _H_BENCHMARK_

void benchmark_display_banner(const char* name, int runs, const char* infos);
long long benchmark_get_time_ns(void);
void benchmark_display_results(long long startt, long long endt, int runs);

#ifdef BENCHMARK_IMPL

#include <time.h>

long long benchmark_get_time_ns() {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return tp.tv_sec * 10e9 + tp.tv_nsec;
}

#include <stdio.h>

void benchmark_display_banner(const char* name, int runs, const char* infos) {
    fprintf(stdout, "Benchmarking `%s`:\n", name);
    fprintf(stdout, "  Runs: %6d, Infos: %s\n", runs, infos);
}

void benchmark_display_results(long long startt, long long endt, int runs) {
    double elapsed = (double)(endt - startt) / 10e9; // s
    double per_run = (double)(endt - startt) / 10e6 / runs; // ms
    fprintf(stdout, "  Runs: %6d, Time elapsed: %6.3lf s, Time/Run: %6.1lf ms\n", runs, elapsed, per_run);
}

#endif

#endif
