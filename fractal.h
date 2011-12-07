#ifndef H_FRACTAL
#define H_FRACTAL

struct fractal;

struct fractal* fractal_create(int width, int height, int bpp);
void fractal_destroy(struct fractal*);
void fractal_clear(struct fractal*);
void fractal_display(struct fractal*);

#endif
