#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL/SDL.h>

#include "config.h"
#include "debug.h"
#include "fractal.h"
#include "mandelbrot.h"

#define GEN_NAME(g) #g

int main(int argc, char* argv[])
{
  debug("*** %s ***", NAME);
  debug("Init...");
  
  SDL_Init(SDL_INIT_VIDEO);
  SDL_WM_SetCaption(NAME, NULL);

  debug("Create fractal: %ix%ix%i, generator: %s", WIDTH, HEIGHT, BPP, GEN_NAME(mandelbrot));
  struct fractal* f = fractal_create(WIDTH, HEIGHT, BPP, mandelbrot);

  debug_separator();

  debug("Main loop");
  
  /* Main loop */
  bool quit = false;
  bool update = true;
  SDL_Event	event;
  while(!quit)
  {
    /* Display */
    if(update && !quit)
    {
      fractal_update(f);
      fractal_display(f);
    }

    // raz
    update = true;

    /* Events */
    SDL_WaitEvent(&event); // blocking
    switch(event.type)
    {
      case SDL_QUIT:
        quit = true;
        debug("Event: SDL_QUIT.");
        break;

      case SDL_KEYDOWN:
        switch(event.key.keysym.sym)
        {
          case SDLK_ESCAPE:
            quit = true;
            debug("Event: SDLK_ESCAPE.");
            break;

          case SDLK_u:
            debug("Event: SDLK_u.");
            break;

          default: update = false; break;
        }
        break;

      default: update = false; break;
    }
  }

  debug_separator();
  debug("Quit...");

  fractal_destroy(f);

  SDL_Quit();

  debug("Goodbye :)");

  return EXIT_SUCCESS;
}
