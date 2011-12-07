#include "fractal.h"

#include <SDL/SDL.h>

#define FCHECK(f,r) \
  if(!f) \
    return r;

struct fractal
{
  SDL_Surface* screen;
};

struct fractal* fractal_create(int width, int height, int bpp)
{
  struct fractal* f = malloc(sizeof(*f));

  FCHECK(f,NULL);

  f->screen = SDL_SetVideoMode(width, height, bpp, SDL_HWSURFACE);

  fractal_clear(f);

  return f;
}

void fractal_destroy(struct fractal* f)
{
  FCHECK(f,);

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

  SDL_Flip(f->screen);
}
