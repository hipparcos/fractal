#include "julia.h"

#include <math.h>
#include <complex.h>

#include "../config.h"

struct color julia(
    int x, int y, int w, int h,
    double lx, double ly,
    double lxmin, double lxmax, double lymin, double lymax,
    int imax)
{
  /* Initialisation */
  int n = 0; /* iterator */
  int colorf = (int)floor(255/imax); /* depth */
  double zrealpart = lx;
  double zimgpart = ly;
  double zmodule = 0;
  double tmprp = 0;

  /* Algorythm */
  while ((zmodule < 4) && (n < imax))
  {
    tmprp = zrealpart;
    zrealpart = (zrealpart * zrealpart) - (zimgpart * zimgpart) + creal(C);
    zimgpart = (2 * tmprp * zimgpart) + cimag(C);
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

