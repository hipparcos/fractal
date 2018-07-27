out=fractal
sources=main.c types.c panic.c renderer_software.c renderer_hardware.c \
		generator/julia_multiset.c generator/julia.c generator/mandelbrot.c
build_dir:=build

objects=$(addprefix $(build_dir)/,$(sources:%.c=%.o))
deps=$(addprefix $(build_dir)/,$(sources:%.c=%.d))

CC=gcc
SHELL:=/bin/bash
DEBUG?=-ggdb3 -O0
CFLAGS:=-Wall -std=c11 $(DEBUG)
LDFLAGS:=-Wall -zmuldefs -lpopt -lSDL2 -lGL -lGLEW -lm
VGFLAGS?=\
	--quiet --leak-check=full --show-leak-kinds=all \
	--track-origins=yes --error-exitcode=1 --error-limit=no \
	--suppressions=./valgrind-libraryleaks.supp

# Use second expansion to create $(build_dir) on demand.
.SECONDEXPANSION:

all: build

build: $(out)

clean:
	rm -f $(objects) $(deps) tags $(out)

leakcheck: $(out)
	valgrind $(VGFLAGS) ./$^

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
