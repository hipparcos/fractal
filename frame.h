#ifndef H_FRAME
#define H_FRAME

#include "debug.h"

#include <stdlib.h>
#include <math.h>

struct frame
{
  double xmin;
  double xmax;
  double ymin;
  double ymax;
  double ratio;
};

inline void frame_reset(struct frame* f)
{
  f->xmin = 0;
  f->xmax = 0;
  f->ymin = 0;
  f->ymax = 0;
}

inline struct frame* frame_create()
{
  struct frame* f = malloc(sizeof(*f));

  frame_reset(f);

  return f;
}

inline void frame_destroy(struct frame* f)
{
  free(f);
}

inline void frame_translate(struct frame* f, double x, double y)
{
  f->xmin += x;
  f->xmax += x;
  f->ymin += y;
  f->ymax += y;
}

inline void frame_set3(struct frame* f, double xmin, double xmax, double ymin)
{
  f->xmin = xmin;
  f->xmax = xmax;
  f->ymin = ymin;
  f->ymax = ymin + (xmax - xmin) * f->ratio;

  debug("Frame: %.6fx%.6fx%.6fx%.6f", xmin, xmax, ymin, f->ymax);
}

inline double frame_width(struct frame* f)
{
  return f->xmax - f->xmin;
}

inline double frame_height(struct frame* f)
{
  return f->ymax - f->ymin;
}

inline void frame_zoom(struct frame* f, double factor)
{
  if(fpclassify(factor) != FP_NORMAL)
    return;

  double xmax,xmin,ymin;
  double z = (1 - ((factor > .0) ? 1/factor : -factor)) / 2;

  xmin = f->xmin + frame_width(f) * z;
  xmax = f->xmax - frame_width(f) * z;
  ymin = f->ymin + frame_height(f) * z;

  frame_set3(f, xmin, xmax, ymin);
}

#endif
