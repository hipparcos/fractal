#include "renderer_software.c"

#include "benchmark_sw_worker.h"

int main(void)
{
    benchmark_worker(rdr_sw_line_worker, RUNS, WIDTH, HEIGHT);

    return EXIT_SUCCESS;
}
