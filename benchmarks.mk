benchmarks_sources:=benchmark_sw_line_worker.c benchmark_sw_area_worker.c
benchmark_build_dir:=$(build_dir)

benchmarks:=$(benchmarks_sources:%.c=%)
benchmarks_obj:=$(addprefix $(build_dir)/,$(benchmarks_sources:%.c=%.o))
benchmarks_built:=$(addprefix $(build_dir)/,$(benchmarks_sources:%.c=%))

RUNS?=1000
BENCH_CFLAGS:=-DRUNS=$(RUNS)

benchmark: $(benchmarks)

$(benchmarks): %: benchmark_clean $(build_dir)/%

$(benchmarks_built): %: %.o $(objects_no_main)
	@$(CC) $(LDFLAGS) $(LDLIBS) $^ -o $@
	@./$@
	@echo ""


$(build_dir)/benchmark_%.o: benchmark_%.c $$(@D)/.f
	@$(CC) $(CFLAGS) $(BENCH_CFLAGS) -c -o $@ $<

benchmark_clean:
	@rm -f $(benchmarks_obj) $(benchmarks_built)

clean:: benchmark_clean

# List of all special targets (always out-of-date).
.PHONY: clean benchmark $(benchmarks) $(benchmarks_built)
