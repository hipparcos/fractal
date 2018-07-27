# fractal

A simple fractal drawer written in C (C11 revision) using SDL2 & OpenGL 3.3.

## Getting started

### Prerequisites

Building:

- GNU Compiler Collection (gcc);
- GNU Make (make);
- popt library: CLI flags parsing;
- SDL2: windowing + software rendering;
- OpenGL 3.3: hardware rendering;
- GLEW: OpenGL extension loader.

### Building & running

```bash
git clone https://github.com/hipparcos/fractal.git
cd fractal
make
./fractal
```

To print CLI arguments usage:
```bash
./fractal -?
```

## Features

fractal renders julia and mandelbrot fractals.

## Commands

```bash
escape or q     quit
u               update display
r               reset display
s               switch generator
arrows          move
p or +          max iterations per pixel + 10 (depth)
m or -          max iterations per pixel - 10 (depth)
CTRL+`+`        zoom+ x1.1
CTRL+`-`        zoom- x1.1
click + drag    zoom (box)
Middle + drag   move
```

## CLI arguments

```
Usage: fractal [OPTIONS]
  -w, --width=INT                              Set window width (default: 800)
  -h, --height=INT                             Set window height (default: 600)
  -z, --zoom=DOUBLE                            Set zoom factor based on screen size (default: 1.1)
  -t, --translate=DOUBLE                       Set translation factor based on screen size (default: 0.25)
  -i, --iter=INT                               Set max iteration limit (default: 50)
      --step=INT                               Set max iteration (incr|decr)ementation step (default: 10)
  -g, --generator=mandelbrot|julia|juliams     Set fractal generator (default: mandelbrot)
      --speed=DOUBLE                           Set dynamic fractals rendering speed (default: 1)
  -s, --software=0|1                           Use software renderer (hardware renderer by default) (default: 0)

Help options:
  -?, --help                                   Show this help message
      --usage                                  Display brief usage message``bash
```
