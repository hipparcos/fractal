# fractal

A simple fractal drawer written in C (C11 revision) using SDL2 & OpenGL 3.3.

![Fractal Julia Multi-Set Dynamic](https://github.com/hipparcos/fractal/blob/master/fractal.gif)

**Output for:**
```bash
./fractal -w 640 -h 480 --preset 4 --speed 2.0
```

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
git submodule init
git submodule update
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
Space           pause (dynamic only)
a               increase speed
d               decrease speed
```

## CLI arguments

```
Usage: fractal [OPTIONS]
  -c, --config               Set config file (default: "config.toml")
  -w, --width=INT            Set window width in pixels
  -h, --height=INT           Set window height in pixels
  -z, --zoom=DOUBLE          Set zoom factor (density per pixel multiplier)
  -t, --translate=DOUBLE     Set translation factor (screen size multiplier)
  -i, --iter=INT             Set max iteration limit
      --step=INT             Set max iteration (incr|decr)ementation step
  -p, --preset=INT           Set fractal preset to use (index of presets, from 0)
      --speed=DOUBLE         Set dynamic fractals rendering speed
  -s, --software=0|1         Use software renderer (hardware renderer by default)

Help options:
  -?, --help                 Show this help message
      --usage                Display brief usage message
```

## TODO

- [ ] Add a GUI (vurtun/nuklear);
- [x] Make software renderer concurrent (pthread);
- [x] Store presets in a config file (cktan/tomlc99).
