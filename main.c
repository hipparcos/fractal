#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL/SDL.h>

#include "debug.h"
#include "fractal.h"

#define NAME "fractal"
#define WIDTH  800
#define HEIGHT 600
#define BPP    32

int main(int argc, char* argv[])
{
  debug("*** %s ***", NAME);
  debug("Init...");
  
  SDL_Init(SDL_INIT_VIDEO);
  SDL_WM_SetCaption(NAME, NULL);

  struct fractal* f = fractal_create(WIDTH, HEIGHT, BPP);

  debug_separator();

  debug("Main loop");
  
  /* Main loop */
  bool quit = false;
  SDL_Event	event;
  while(!quit)
  {
    /* Events */
    SDL_WaitEvent(&event);
    switch(event.type)
    {
      case SDL_QUIT:
        quit = true;
        debug("Event : SDL_QUIT.");
        break;

      case SDL_KEYDOWN:
        switch(event.key.keysym.sym)
        {
          case SDLK_ESCAPE:
            quit = true;
            debug("Event : SDLK_ESCAPE.");
            break;

          default: break;
        }
        break;

      default: break;
    }

    /* Display */
    fractal_display(f);
  }

  debug_separator();
  debug("Quit...");

  fractal_destroy(f);

  SDL_Quit();

  debug("Goodbye :)");

  return EXIT_SUCCESS;
}
