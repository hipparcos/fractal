# fractal

A simple fractal drawer written in C (C11 revision) using SDL2.

## Getting started

### Prerequisites

Building:

- GNU Compiler Collection (gcc);
- GNU Make (make);
- popt library: CLI flags parsing;
- SDL2: rendering.

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
escape or q   quit
u             update display
r             reset display
arrows        move
p or +        max iterations per pixel + 10 (depth)
m or -        max iterations per pixel - 10 (depth)
CTRL+`+`      zoom+ x1.1
CTRL+`-`      zoom- x1.1
click + drag  zoom (box)
```
