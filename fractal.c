#include "fractal.h"

#include <SDL/SDL.h>

#include "debug.h"

#define FCHECK(f,r) \
  if(!f) \
    return r;

struct fractal
{
  SDL_Surface* screen;
  SDL_Surface* buffer;
  fractal_generator generator;
};

struct fractal* fractal_create(int width, int height, int bpp, fractal_generator gen)
{
  struct fractal* f = malloc(sizeof(*f));

  FCHECK(f,NULL);

  f->screen = SDL_SetVideoMode(width, height, bpp, SDL_HWSURFACE);
  f->buffer = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, bpp, 0, 0, 0, 0);
  f->generator = gen;

  fractal_clear(f);

  return f;
}

void fractal_destroy(struct fractal* f)
{
  FCHECK(f,);

  SDL_FreeSurface(f->buffer);
  SDL_FreeSurface(f->screen);
  free(f);
}

void fractal_clear(struct fractal* f)
{
  FCHECK(f,);

  SDL_FillRect(f->screen, NULL, SDL_MapRGB(f->screen->format, 0, 0, 0));
}

void fractal_display(struct fractal* f)
{
  FCHECK(f,);

  debug("Display: flip screen.");
  SDL_Flip(f->screen);
}

void fractal_update(struct fractal* f)
{
  FCHECK(f,);

  debug_separator();

  debug("Update: start.");
  Uint32 tstart = SDL_GetTicks();

  debug("Update: clear buffer.");
  SDL_FillRect(f->buffer, NULL, SDL_MapRGB(f->buffer->format, 0, 0, 0));

  debug("Update: render loop.");
  for(int y = 0; y < f->screen->h; y++)
  {
    for(int x = 0; x < f->screen->w; x++)
    {
      Uint32 color = f->generator(x,y,f->screen->w,f->screen->h,0,0,0,0);
      // Set pixel color.
      *((Uint32*)(f->buffer->pixels) + x + y * f->screen->w) = color;
    }
  }

  debug("Update: blit buffer.");
  SDL_Rect rect;
  rect.x = 0;
  rect.y = 0;
  SDL_BlitSurface(f->buffer, NULL, f->screen, &rect);

  Uint32 tend = SDL_GetTicks();
  debug("Update: updated in %i miliseconds.", tend - tstart);
  
  debug_separator();
}
