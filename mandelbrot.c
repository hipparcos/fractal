#include "mandelbrot.h"

#include "math.h"

struct color mandelbrot(
    int x, int y, int w, int h,
    double lxmin, double lxmax, double lymin, double lymax,
    int imax)
{
  /* Initialisation */
  int n = 0; /* iterator */
  int colorf = (int)floor(255/imax); /* depth */
  double zrealpart = 0;
  double zimgpart = 0;
  double zmodule = 0;
  double tmprp = 0;

  /* Data */
  double const dx = (lxmax - lxmin) / w;
  double const dy = (lymax - lymin) / h;
  double realpart = lxmin + x * dx;
  double imgpart = lymin + y * dy;

  /* Algorythm */
  while ((zmodule < 4) && (n < imax))
  {
    tmprp = zrealpart;
    zrealpart = (zrealpart * zrealpart) - (zimgpart * zimgpart) + realpart;
    zimgpart = (2 * tmprp * zimgpart) + imgpart;
    zmodule = (zrealpart * zrealpart) + (zimgpart * zimgpart);
    n++;
  }

  /* Color */
  int red,green,blue;

  if (n < imax)
  {
    red = green = blue = n * colorf;
  }
  else
  {
    red = green = blue = 0;
  }

  struct color c = {red, green, blue};

  return c;
}

