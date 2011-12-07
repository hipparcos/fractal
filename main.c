#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL/SDL.h>

#include "debug.h"

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
  SDL_Surface* window = SDL_SetVideoMode(WIDTH, HEIGHT, BPP, SDL_HWSURFACE);

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

    /* Clear display */
    SDL_FillRect(window, NULL, SDL_MapRGB(window->format, 0, 0, 0));

    /* Display */
    SDL_Flip(window);
  }

  debug_separator();
  debug("Quit...");

  SDL_FreeSurface(window);
  SDL_Quit();

  debug("Goodbye :)");

  return EXIT_SUCCESS;
}
