out=fractal
sources=main.c config.c types.c panic.c renderer_software.c renderer_hardware.c \
		generator/julia_multiset.c generator/julia.c generator/mandelbrot.c \
		vendor/tomlc99/toml.c
build_dir:=build
benchmark_file:=benchmarks.mk

objects=$(addprefix $(build_dir)/,$(sources:%.c=%.o))
objects_no_main=$(addprefix $(build_dir)/,$(filter-out main.o,$(sources:%.c=%.o)))
deps=$(addprefix $(build_dir)/,$(sources:%.c=%.d))

CC=gcc
SHELL:=/bin/bash
# DEBUG?=-ggdb3 -O0
DEBUG?=-O2
CFLAGS:=-Wall -Wno-unused-function -std=c11 $(DEBUG)
LDFLAGS:=-Wall -zmuldefs
LDLIBS:=-lpopt -lSDL2 -lGL -lGLEW -lm -lpthread
VGFLAGS?=\
	--quiet --leak-check=full --show-leak-kinds=all \
	--track-origins=yes --error-exitcode=1 --error-limit=no \
	--suppressions=./valgrind-libraryleaks.supp

# Use second expansion to create $(build_dir) on demand.
.SECONDEXPANSION:

all: build

build: $(out)

clean::
	rm -f $(objects) $(deps) tags $(out)

leakcheck: $(out)
	valgrind $(VGFLAGS) ./$^

# Benchmarks targets
include $(benchmark_file)

# Build executable.
$(out): $(objects)
	$(CC) $(LDFLAGS) $(LDLIBS) $^ -o $@

# Generate O file in $(build_dir); .f is a directory marker.
$(build_dir)/%.o: %.c $$(@D)/.f
	$(CC) $(CFLAGS) -c -o $@ $<

# Generate C source files dependancies in $(build_dir); .f is a directory marker.
$(build_dir)/%.d: %.c $$(@D)/.f
	@set -e; rm -f $@; \
		$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$

# Include dependancies makefiles.
include $(deps)

# Generate tags file.
tags:
	ctags -R .

# Directory marker.
%/.f:
	@mkdir -p $(dir $@)
	@touch $@

.PRECIOUS: %/.f

# List of all special targets (always out-of-date).
.PHONY: all build clean tags
