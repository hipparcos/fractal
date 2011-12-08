#include <stdlib.h>
#include <stdio.h>

#include "config.h"
#include "debug.h"
#include "fractal.h"
#include "event.h"

/* Generators */
#include "generator/mandelbrot.h"

int main(int argc, char* argv[])
{
  debug("*** %s ***", NAME);

  debug("Init...");
  fractal_env_init(NAME);

  debug("Create fractal: %ix%ix%i, imax: %i", WIDTH, HEIGHT, BPP, IMAX);
  struct fractal* f = fractal_create(WIDTH, HEIGHT, BPP, GENERATOR, IMAX);

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
