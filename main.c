#include <stdlib.h>
#include <stdio.h>

#include "config.h"
#include "debug.h"
#include "fractal.h"
#include "mandelbrot.h"
#include "event.h"

#define GEN_NAME(g) #g

int main(int argc, char* argv[])
{
  debug("*** %s ***", NAME);

  debug("Init...");
  fractal_env_init(NAME);

  debug("Create fractal: %ix%ix%i, generator: %s, imax: %i", WIDTH, HEIGHT, BPP,
      GEN_NAME(mandelbrot), IMAX);
  struct fractal* f = fractal_create(WIDTH, HEIGHT, BPP, mandelbrot, IMAX);

  debug("Set default frame values.");
  struct frame* fm = fractal_get_frame(f);
  frame_set3(fm, XMIN, XMAX, YMIN);

  debug_separator();

  debug("Main loop.");
  event_loop(f);

  debug_separator();
  
  debug("Quit...");
  fractal_destroy(f);
  fractal_env_quit();

  debug("Goodbye :)");

  return EXIT_SUCCESS;
}
