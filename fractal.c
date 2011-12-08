#include "fractal.h"

#include <SDL/SDL.h>

#include "debug.h"
#include "frame.h"

#define FCHECK(f,r) \
  if(!f) \
    return r;

struct fractal
{
  SDL_Surface* screen;
  SDL_Surface* buffer;
  fractal_generator generator;
  struct frame* frame;
  int imax;
};

struct fractal* fractal_create(int width, int height, int bpp, fractal_generator gen, int imax)
{
  struct fractal* f = malloc(sizeof(*f));

  FCHECK(f,NULL);

  f->screen = SDL_SetVideoMode(width, height, bpp, SDL_HWSURFACE);
  f->buffer = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, bpp, 0, 0, 0, 0);
  f->generator = gen;

  fractal_clear(f);

  f->frame = frame_create();
  f->frame->ratio = (double)height/width;
  f->imax = imax;

  return f;
}

void fractal_destroy(struct fractal* f)
{
  FCHECK(f,);

  SDL_FreeSurface(f->buffer);
  SDL_FreeSurface(f->screen);
  frame_destroy(f->frame);
  free(f);
}

struct frame* fractal_get_frame(struct fractal* f)
{
  return f->frame;
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

inline Uint32 color_to_sdl(struct color c, SDL_PixelFormat* fmt)
{
  return SDL_MapRGB(fmt, c.r, c.g, c.b);
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
      // Render a pixel.
      struct color c = f->generator(
          x,y,f->screen->w,f->screen->h,
          f->frame->xmin,f->frame->xmax,f->frame->ymin,f->frame->ymax,
          f->imax);
      // Set pixel color.
      *((Uint32*)(f->buffer->pixels) + x + y * f->screen->w)
        = color_to_sdl(c, f->buffer->format);
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
