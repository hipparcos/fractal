#include "event.h"

#include <stdbool.h>
#include <SDL/SDL.h>

#include "debug.h"
#include "config.h"

void event_loop(struct fractal* f)
{
  bool quit = false;
  bool update = true;
  SDL_Event	event;
  Uint16 mbpx = 0, mbpy = 0, mbrx = 0, mbry = 0;
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
            frame_translate(fm, .0, -(frame_height(fm)*TRANSLATE_FACTOR));
            debug("Event: move up.");
            break;
          case SDLK_DOWN:
            frame_translate(fm, .0, frame_height(fm)*TRANSLATE_FACTOR);
            debug("Event: move down.");
            break;
          case SDLK_RIGHT:
            frame_translate(fm, frame_width(fm)*TRANSLATE_FACTOR, .0);
            debug("Event: move right.");
            break;
          case SDLK_LEFT:
            frame_translate(fm, -(frame_width(fm)*TRANSLATE_FACTOR), .0);
            debug("Event: move left.");
            break;

          case SDLK_KP_PLUS:
            if((event.key.keysym.mod & KMOD_LCTRL) == KMOD_LCTRL ||
                (event.key.keysym.mod & KMOD_RCTRL) == KMOD_RCTRL)
            {
              frame_zoom(fm, ZOOM_FACTOR);
              debug("Event: zoom+ x%.1f", ZOOM_FACTOR);
            }
            else
            {
              if(fractal_get_imax(f) < 240)
              {
                fractal_set_imax(f, fractal_get_imax(f) + 10);
                debug("Event: increase imax by 10 to %d.", fractal_get_imax(f));
              }
              else
                update = false;
            }
            break;
          case SDLK_KP_MINUS:
            if((event.key.keysym.mod & KMOD_LCTRL) == KMOD_LCTRL ||
                (event.key.keysym.mod & KMOD_RCTRL) == KMOD_RCTRL)
            {
              frame_zoom(fm, -ZOOM_FACTOR);
              debug("Event: zoom- x%.1f", ZOOM_FACTOR);
            }
            else
            {
              if(fractal_get_imax(f) > 10)
              {
                fractal_set_imax(f, fractal_get_imax(f) - 10);
                debug("Event: decrease imax by 10 to %d.", fractal_get_imax(f));
              }
              else
                update = false;
            }
            break;

          case SDLK_r:
            debug("Event: reset.");
            fractal_set_imax(f, IMAX);
            frame_set3(fm, XMIN, XMAX, YMIN);
            break;

          default: update = false; break;
        }
        break;

      case SDL_MOUSEBUTTONDOWN:
        switch(event.button.button)
        {
          case SDL_BUTTON_LEFT:
            mbpx = event.button.x;
            mbpy = event.button.y;
            update = false;
            break;
        }
        break;
      
      case SDL_MOUSEBUTTONUP:
        switch(event.button.button)
        {
          case SDL_BUTTON_LEFT:
            debug("Event: zoom (mouse).");
            mbrx = event.button.x;
            mbry = event.button.y;

            if(mbrx != mbpx && mbry != mbpy)
            {
              double xmax,xmin,ymin;

              if(mbrx > mbpx)
              {
                xmin = fractal_globalx_to_localx(f, mbpx);
                xmax = fractal_globalx_to_localx(f, mbrx);
              }
              else
              {
                xmin = fractal_globalx_to_localx(f, mbrx);
                xmax = fractal_globalx_to_localx(f, mbpx);
              }

              ymin = fractal_globaly_to_localy(f, mbpy);

              frame_set3(fm, xmin, xmax, ymin);
            }
            break;
        }
        break;

      default: update = false; break;
    }
  }
}
