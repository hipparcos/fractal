#include "event.h"

#include <stdbool.h>
#include <SDL/SDL.h>

#include "debug.h"

void event_loop(struct fractal* f)
{
  bool quit = false;
  bool update = true;
  SDL_Event	event;
  struct frame* fm = fractal_get_frame(f);

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

          case SDLK_UP:
            frame_translate(fm, .0, -(frame_height(fm)/4));
            debug("Event: move up.");
            break;
          case SDLK_DOWN:
            frame_translate(fm, .0, frame_height(fm)/4);
            debug("Event: move down.");
            break;
          case SDLK_RIGHT:
            frame_translate(fm, frame_width(fm)/4, .0);
            debug("Event: move right.");
            break;
          case SDLK_LEFT:
            frame_translate(fm, -(frame_width(fm)/4), .0);
            debug("Event: move left.");
            break;

          default: update = false; break;
        }
        break;

      default: update = false; break;
    }
  }
}
