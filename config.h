#ifndef H_CONFIG
#define H_CONFIG

#define NAME "fractal"
#define WIDTH  800
#define HEIGHT 600
#define BPP 32
#define ZOOM_FACTOR 2.0
#define TRANSLATE_FACTOR 0.25

#define JULIA

#ifdef MANDELBROT
#define GENERATOR mandelbrot
#define XMIN -2.1
#define XMAX 0.7
#define YMIN -1.05
#define IMAX 50
#endif

#ifdef JULIA
#define GENERATOR julia
#define XMIN -1.7
#define XMAX 1.7
#define YMIN -1.25
#define IMAX 50
#define C -0.8+0.156*I
//#define C -0.4+0.6*I
//#define C -0.70176-0.3842*I
//#define C 0.285+0.01*I
#endif

#endif
