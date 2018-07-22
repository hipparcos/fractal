#include "frame.h"

#include <math.h>

void frame_reset(struct frame* fm) {
    fm->xmin = 0;
    fm->xmax = 0;
    fm->ymin = 0;
    fm->ymax = 0;
}


void frame_copy(struct frame* dest, const struct frame* src) {
    dest->xmin = src->xmin;
    dest->xmax = src->xmax;
    dest->ymin = src->ymin;
    frame_set_ymax(dest);
}

void frame_set(struct frame* fm, double xmin, double xmax, double ymin) {
    fm->xmin = xmin;
    fm->xmax = xmax;
    fm->ymin = ymin;
    frame_set_ymax(fm);
}

void frame_set_ymax(struct frame* fm) {
    fm->ymax = fm->ymin + (fm->xmax - fm->xmin) * (1/fm->ratio);
}

double frame_width(struct frame* fm) {
    return fm->xmax - fm->xmin;
}

double frame_height(struct frame* fm) {
    return fm->ymax - fm->ymin;
}

void frame_translate(struct frame* fm, double x, double y) {
    fm->xmin += x;
    fm->xmax += x;
    fm->ymin += y;
    fm->ymax += y;
}

void frame_zoom(struct frame* fm, double factor) {
    if(fpclassify(factor) != FP_NORMAL) {
        return;
    }

    double xmax, xmin, ymin;
    double z = (1 - ((factor > .0) ? 1/factor : -factor)) / 2;

    xmin = fm->xmin + frame_width(fm) * z;
    xmax = fm->xmax - frame_width(fm) * z;
    ymin = fm->ymin + frame_height(fm) * z;

    frame_set(fm, xmin, xmax, ymin);
}

double frame_globalx_to_localx(struct frame* fm, int x, int width) {
    return fm->xmin + (fm->xmax - fm->xmin) * ((double)x / width);
}

double frame_globaly_to_localy(struct frame* fm, int y, int height) {
    return fm->ymin + (fm->ymax - fm->ymin) * ((double)y / height);
}
