benchmarks_sources:=benchmark_sw_line_worker.c benchmark_sw_area_worker.c
benchmark_build_dir:=$(build_dir)

benchmarks:=$(benchmarks_sources:%.c=%)
benchmarks_obj:=$(addprefix $(build_dir)/,$(benchmarks_sources:%.c=%.o))
benchmarks_built:=$(addprefix $(build_dir)/,$(benchmarks_sources:%.c=%))

BENCH_CFLAGS:=

benchmark: $(benchmarks)

$(benchmarks): %: $(build_dir)/%

$(benchmarks_built): %: %.o $(objects_no_main)
	@$(CC) $(LDFLAGS) $(LDLIBS) $^ -o $@
	time ./$@


$(build_dir)/benchmark_%.o: benchmark_%.c $$(@D)/.f
	@$(CC) $(CFLAGS) $(BENCH_CFLAGS) -c -o $@ $<

clean::
	rm -f $(benchmarks_obj) $(benchmarks_built)

# List of all special targets (always out-of-date).
.PHONY: clean benchmark $(benchmarks) $(benchmarks_built)
