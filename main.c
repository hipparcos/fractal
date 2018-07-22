#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

#include "fractal.h"
#include "generator/mandelbrot.h"
#include "generator/julia.h"

static char*  title      = "fractal";
static int    width      = 800;
static int    height     = 600;
static int    bpp        = 32;
static double zoom       = 2.0;
static double translate  = 0.25;
static int    resolution = 50;

static struct frame frm_mandelbrot = {
    .xmin= -2.1,
    .xmax=  0.7,
    .ymin= -1.05,
};
static struct frame frm_julia = {
    .xmin= -1.7,
    .xmax=  1.7,
    .ymin= -1.25,
};

void event_loop(struct fractal* f, struct frame* fm, double zoom, double translate);

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_WM_SetCaption(title, NULL);

    struct win_info wi = {
        .width=  width,
        .height= height,
        .bpp=    bpp,
    };
    struct fractal_info fi = {
        .generator=     mandelbrot,
        .default_frame= frm_mandelbrot,
        .resolution=    resolution
    };
    struct frame fm = fi.default_frame;
    fm.ratio = (double)(width)/height;
    frame_set_ymax(&fm);

    struct fractal* f = fractal_create(wi, fi);

    event_loop(f, &fm, zoom, translate);

    fractal_destroy(f);
    SDL_Quit();

    return EXIT_SUCCESS;
}

void event_loop(struct fractal* f, struct frame* fm, double zoom, double translate) {
    bool quit = false;
    bool update = true;
    SDL_Event event;
    Uint16 mbpx = 0, mbpy = 0, mbrx = 0, mbry = 0;
    struct frame df = {0};

    while(!quit) {
        /* Display */
        if(update && !quit) {
            fractal_update(f, fm);
            fractal_display(f);
        }

        update = true;

        /* Events */
        SDL_WaitEvent(&event); // blocking
        switch(event.type) {
        case SDL_QUIT:
            quit = true;
            break;

        case SDL_KEYDOWN:
            switch(event.key.keysym.sym) {
            case SDLK_ESCAPE:
                quit = true;
                break;

            case SDLK_u:
                break;

            case SDLK_UP:
                frame_translate(fm, .0, -(frame_height(fm) * translate));
                break;
            case SDLK_DOWN:
                frame_translate(fm, .0, frame_height(fm) * translate);
                break;
            case SDLK_RIGHT:
                frame_translate(fm, frame_width(fm) * translate, .0);
                break;
            case SDLK_LEFT:
                frame_translate(fm, -(frame_width(fm) * translate), .0);
                break;

            case SDLK_p:
            case SDLK_PLUS:
            case SDLK_KP_PLUS:
                if((event.key.keysym.mod & KMOD_LCTRL) == KMOD_LCTRL ||
                        (event.key.keysym.mod & KMOD_RCTRL) == KMOD_RCTRL) {
                    frame_zoom(fm, zoom);
                } else {
                    if(fractal_get_imax(f) < 240) {
                        fractal_set_imax(f, fractal_get_imax(f) + 10);
                    } else {
                        update = false;
                    }
                }
                break;
            case SDLK_m:
            case SDLK_MINUS:
            case SDLK_KP_MINUS:
                if((event.key.keysym.mod & KMOD_LCTRL) == KMOD_LCTRL ||
                        (event.key.keysym.mod & KMOD_RCTRL) == KMOD_RCTRL) {
                    frame_zoom(fm, -zoom);
                } else {
                    if(fractal_get_imax(f) > 10) {
                        fractal_set_imax(f, fractal_get_imax(f) - 10);
                    } else {
                        update = false;
                    }
                }
                break;

            case SDLK_r:
                df = fractal_get_default_frame(f);
                frame_copy(fm, &df);
                fractal_set_imax(f, fractal_get_default_resolution(f));
                break;

            default:
                update = false;
                break;
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
            switch(event.button.button) {
            case SDL_BUTTON_LEFT:
                mbpx = event.button.x;
                mbpy = event.button.y;
                update = false;
                break;
            }
            break;

        case SDL_MOUSEBUTTONUP:
            switch(event.button.button) {
            case SDL_BUTTON_LEFT:
                mbrx = event.button.x;
                mbry = event.button.y;

                if(mbrx != mbpx && mbry != mbpy) {
                    double xmax,xmin,ymin;

                    if(mbrx > mbpx) {
                        xmin = frame_globalx_to_localx(fm, mbpx, fractal_get_width(f));
                        xmax = frame_globalx_to_localx(fm, mbrx, fractal_get_width(f));
                    } else {
                        xmin = frame_globalx_to_localx(fm, mbrx, fractal_get_width(f));
                        xmax = frame_globalx_to_localx(fm, mbpx, fractal_get_width(f));
                    }

                    ymin = frame_globaly_to_localy(fm, mbpy, fractal_get_height(f));

                    frame_set(fm, xmin, xmax, ymin);
                }
                break;
            }
            break;

        default:
            update = false;
            break;
        }
    }
}
