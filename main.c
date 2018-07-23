#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <limits.h>
#include <popt.h>

#include "fractal.h"
#include "generator/mandelbrot.h"
#include "generator/julia.h"

static char*  title      = "fractal";
static int    width      = 800;
static int    height     = 600;
static int    bpp        = 32;
static double zoom       = 1.1;
static double translate  = 0.25;
static int    max_iter   = 50;

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

void panic(const char* err) {
    fputs(err, stderr);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}

#define LENGTH(arr) sizeof(arr)/sizeof(arr[0])

int main(int argc, char* argv[]) {
    /* Default config. */
    struct win_info wi = {
        .title=  title,
        .width=  width,
        .height= height,
        .bpp=    bpp,
    };
    struct fractal_info fi[] = {
        {
            .generator=     mandelbrot,
            .default_frame= frm_mandelbrot,
            .max_iter=      max_iter,
        },
        {
            .generator=     julia,
            .default_frame= frm_julia,
            .max_iter=      max_iter,
        },
    };
    size_t ifi = 0;

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

    /* Set max_iter in fi. */
    for (size_t i = 0; i < LENGTH(fi); i++) {
        fi[i].max_iter = max_iter;
    }

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
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        panic("Error: SDL can't create a renderer.");
    }
    SDL_Texture* texture = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            wi.width, wi.height);
    if (!texture) {
        panic("Error: SDL can't create a texture.");
    }


    struct frame fm = fi[ifi].default_frame;
    fm.ratio = (double)(width)/height;
    frame_set_ymax(&fm);

    struct fractal* f = fractal_create(wi, fi[ifi]);
    if (!f) {
        panic("Error: can't create fractal.");
    }

    /* Main loop. */
    bool quit = false;
    bool update = true;
    SDL_Event event;
    Uint16 mbpx = 0, mbpy = 0, mbrx = 0, mbry = 0;

    while(!quit) {
        /* Display */
        if(update && !quit) {
            fractal_update(f, &fm);
            Uint32* pixels = (Uint32*) fractal_get_pixels(f);
            SDL_UpdateTexture(texture, NULL, pixels, wi.width * sizeof(Uint32));
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
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
                frame_translate(&fm, .0, -(frame_height(&fm) * translate));
                break;
            case SDLK_DOWN:
                frame_translate(&fm, .0, frame_height(&fm) * translate);
                break;
            case SDLK_RIGHT:
                frame_translate(&fm, frame_width(&fm) * translate, .0);
                break;
            case SDLK_LEFT:
                frame_translate(&fm, -(frame_width(&fm) * translate), .0);
                break;

            case SDLK_p:
            case SDLK_PLUS:
            case SDLK_KP_PLUS:
                if((event.key.keysym.mod & KMOD_LCTRL) == KMOD_LCTRL ||
                        (event.key.keysym.mod & KMOD_RCTRL) == KMOD_RCTRL) {
                    frame_zoom(&fm, zoom);
                } else {
                    fractal_max_iter_incr(f, 10);
                }
                break;
            case SDLK_m:
            case SDLK_MINUS:
            case SDLK_KP_MINUS:
                if((event.key.keysym.mod & KMOD_LCTRL) == KMOD_LCTRL ||
                        (event.key.keysym.mod & KMOD_RCTRL) == KMOD_RCTRL) {
                    frame_zoom(&fm, -zoom);
                } else {
                    fractal_max_iter_decr(f, 10);
                }
                break;

            /* Switch. */
            case SDLK_s:
                ifi += 1;
                ifi %= LENGTH(fi);
            /* Reset. */
            case SDLK_r:
                frame_copy(&fm, &fi[ifi].default_frame);
                fractal_set_generator(f, fi[ifi].generator);
                fractal_set_max_iter(f, fi[ifi].max_iter);
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
                        xmin = frame_globalx_to_localx(&fm, mbpx, wi.width);
                        xmax = frame_globalx_to_localx(&fm, mbrx, wi.width);
                    } else {
                        xmin = frame_globalx_to_localx(&fm, mbrx, wi.width);
                        xmax = frame_globalx_to_localx(&fm, mbpx, wi.width);
                    }

                    ymin = frame_globaly_to_localy(&fm, mbpy, wi.height);

                    frame_set(&fm, xmin, xmax, ymin);
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
    fractal_destroy(f);
    SDL_Quit();

    return EXIT_SUCCESS;
}

