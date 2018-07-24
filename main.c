#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <limits.h>
#include <popt.h>

#include "renderer_software.h"
#include "panic.h"
#include "types.h"

/* CLI arguments default. */
static char*  title     = "fractal";
static int    width     = 800;
static int    height    = 600;
static double zoom      = 1.1;
static double translate = 0.25;
static unsigned long max_iter = 50;

void max_iter_incr(unsigned long* max_iter, unsigned long step) {
    if (*max_iter > ULONG_MAX - step) {
        *max_iter = ULONG_MAX;
    } else {
        *max_iter += step;
    }
}

void max_iter_decr(unsigned long* max_iter, unsigned long step) {
    if (*max_iter < step) {
        *max_iter = 0;
    } else {
        *max_iter -= step;
    }
}

#define LENGTH(arr) sizeof(arr)/sizeof(arr[0])

int main(int argc, char* argv[]) {
    /* Default config. */
    struct win_info wi = {
        .title=  title,
        .width=  width,
        .height= height,
    };
    struct fractal_info fi[] = {
        {
            .generator= GEN_MANDELBROT,
            .xmin= -2.1,
            .xmax=  0.7,
            .ymin= -1.05,
        },
        {
            .generator= GEN_JULIA,
            .xmin= -1.7,
            .xmax=  1.7,
            .ymin= -1.25,
        },
    };
    size_t ifi = 0;
    /* Init renderer to software renderer. */
    struct renderer renderer = {0};
    renderer.set_generator   = rdr_sw_set_generator;
    renderer.set_frame       = rdr_sw_set_frame;
    renderer.translate       = rdr_sw_translate;
    renderer.zoom            = rdr_sw_zoom;
    renderer.render          = rdr_sw_render;

    /* CLI arguments. */
    struct poptOption optionsTable[] = {
        {"width", 'w', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,
            &wi.width, 0, "Set window width", NULL},
        {"height", 'h', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,
            &wi.height, 0, "Set window height", NULL},
        {"zoom", 'z', POPT_ARG_DOUBLE|POPT_ARGFLAG_SHOW_DEFAULT,
            &zoom, 0, "Set zoom factor based on screen size", NULL},
        {"translate", 't', POPT_ARG_DOUBLE|POPT_ARGFLAG_SHOW_DEFAULT,
            &translate, 0, "Set translation factor based on screen size", NULL},
        {"iter", 'i', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,
            &max_iter, 0, "Set max iteration limit", NULL},
        {"generator", 'g', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,
            NULL, 'g', "Set fractal generator", "mandelbrot|julia"},
        POPT_AUTOHELP
        POPT_TABLEEND
    };

    poptContext optCon = poptGetContext(NULL, argc, (const char**) argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]");

    char c;
    char* gen;
    while ((c = poptGetNextOpt(optCon)) >= 0) {
        switch (c) {
            case 'g':
                gen = poptGetOptArg(optCon);
                if (strcmp(gen, "julia") == 0) {
                    ifi = 1;
                } else {
                    ifi = 0;
                }
                break;
            default:
                poptPrintUsage(optCon, stderr, 0);
                exit(EXIT_FAILURE);
        }
    }
    if (c < -1) {
        /* an error occurred during option processing */
        fprintf(stderr, "%s: %s\n",
                poptBadOption(optCon, POPT_BADOPTION_NOALIAS),
                poptStrerror(c));
        exit(EXIT_FAILURE);
    }
    poptFreeContext(optCon);

    /* Init. */
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow(wi.title,
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            wi.width, wi.height,
            SDL_WINDOW_OPENGL);
    if (!window) {
        panic("Error: SDL can't open a window.");
    }

    rdr_sw_init(window, fi[ifi]);

    /* Main loop. */
    bool quit = false;
    bool update = true;
    SDL_Event event;
    Uint16 mbpx = 0, mbpy = 0, mbrx = 0, mbry = 0;

    while(!quit) {
        /* Display */
        if (update) {
            renderer.render(max_iter);
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
            case SDLK_q:
            case SDLK_ESCAPE:
                quit = true;
                break;

            case SDLK_u:
                break;

            case SDLK_UP:
                renderer.translate(0, -translate);
                break;
            case SDLK_DOWN:
                renderer.translate(0,  translate);
                break;
            case SDLK_RIGHT:
                renderer.translate( translate, 0);
                break;
            case SDLK_LEFT:
                renderer.translate(-translate, 0);
                break;

            case SDLK_p:
            case SDLK_PLUS:
            case SDLK_KP_PLUS:
                if((event.key.keysym.mod & KMOD_LCTRL) == KMOD_LCTRL ||
                        (event.key.keysym.mod & KMOD_RCTRL) == KMOD_RCTRL) {
                    renderer.zoom(zoom);
                } else {
                    max_iter_incr(&max_iter, 10);
                }
                break;
            case SDLK_m:
            case SDLK_MINUS:
            case SDLK_KP_MINUS:
                if((event.key.keysym.mod & KMOD_LCTRL) == KMOD_LCTRL ||
                        (event.key.keysym.mod & KMOD_RCTRL) == KMOD_RCTRL) {
                    renderer.zoom(1/zoom);
                } else {
                    max_iter_decr(&max_iter, 10);
                }
                break;

            /* Switch. */
            case SDLK_s:
                ifi += 1;
                ifi %= LENGTH(fi);
            /* Reset. */
            case SDLK_r:
                renderer.set_frame(fi[ifi].xmin, fi[ifi].xmax, fi[ifi].ymin);
                renderer.set_generator(fi[ifi].generator);
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
                    /* Center the view... */
                    int center_x = wi.width / 2;
                    int center_y = wi.height / 2;
                    int new_center_x = (mbrx + mbpx) / 2;
                    int new_center_y = (mbry + mbpy) / 2;
                    double tx = ((double)(new_center_x) - center_x) / wi.width;
                    double ty = ((double)(new_center_y) - center_y) / wi.height;
                    renderer.translate(tx, ty);
                    /* ...then zoom in. */
                    double fx = wi.width / (double)abs(mbrx - mbpx);
                    double fy = wi.height / (double)abs(mbry - mbpy);
                    double fc = (fx < fy) ? fx : fy;
                    renderer.zoom(fc);
                }
                break;
            }
            break;

        default:
            update = false;
            break;
        }
    }

    /* Deinit. */
    rdr_sw_free();
    SDL_Quit();

    return EXIT_SUCCESS;
}

