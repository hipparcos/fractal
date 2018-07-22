# fractal

A simple fractal drawer using SDL.

## Getting started

### Prerequisites

Building:

- GNU Compiler Collection (gcc);
- GNU Make (make);
- GNU C Library (glibc/getopt): CLI flags parsing;
- SDL: rendering.

Testing:

- Valgrind (valgrind): memory leaks detection;

### Building & running

```bash
git clone https://github.com/hipparcos/fractal.git
cd fractal
make
./fractal
```

## Features

fractal renders julia and mandelbrot fractals.

## Commands

```bash
escape        quit
u             update display
r             reset display
arrows        move
p or +        max iterations per pixel + 10 (depth)
m or -        max iterations per pixel - 10 (depth)
CTRL+`+`      zoom+ x2
CTRL+`-`      zoom- x2
click + drag  zoom (box)
```
