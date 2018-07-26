#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <limits.h>
#include <popt.h>

#include "renderer_software.h"
#include "renderer_hardware.h"
#include "panic.h"
#include "types.h"

/* CLI arguments default. */
static char*  title     = "fractal";
static int    width     = 800;
static int    height    = 600;
static double zoom      = 1.1;
static double translate = 0.25;
static int    software  = 1;
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
    struct fractal_info fi[] = {
        {
            .generator= GEN_MANDELBROT,
            .cx=  -0.7,
            .cy=   0.0,
            .dpp=  0.0035,
        },
        {
            .generator= GEN_JULIA,
            .cx=   0.0,
            .cy=   0.0,
            .dpp=  0.00425,
        },
    };
    size_t ifi = 0;

    /* CLI arguments. */
    struct poptOption optionsTable[] = {
        {"width", 'w', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,
            &width, 0, "Set window width", NULL},
        {"height", 'h', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,
            &height, 0, "Set window height", NULL},
        {"zoom", 'z', POPT_ARG_DOUBLE|POPT_ARGFLAG_SHOW_DEFAULT,
            &zoom, 0, "Set zoom factor based on screen size", NULL},
        {"translate", 't', POPT_ARG_DOUBLE|POPT_ARGFLAG_SHOW_DEFAULT,
            &translate, 0, "Set translation factor based on screen size", NULL},
        {"iter", 'i', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,
            &max_iter, 0, "Set max iteration limit", NULL},
        {"generator", 'g', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,
            NULL, 'g', "Set fractal generator", "mandelbrot|julia"},
        {"software", 's', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,
            &software, 0, "Use software renderer", NULL},
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

    /* Select renderer. */
    struct renderer renderer = {0};
    if (software) {
        renderer.init            = rdr_sw_init;
        renderer.free            = rdr_sw_free;
        renderer.set_generator   = rdr_sw_set_generator;
        renderer.set_center      = rdr_sw_set_center;
        renderer.set_dpp         = rdr_sw_set_dpp;
        renderer.translate       = rdr_sw_translate;
        renderer.zoom            = rdr_sw_zoom;
        renderer.resize          = rdr_sw_resize;
        renderer.render          = rdr_sw_render;
    } else {
        renderer.init            = rdr_hw_init;
        renderer.free            = rdr_hw_free;
        renderer.set_generator   = rdr_hw_set_generator;
        renderer.set_center      = rdr_hw_set_center;
        renderer.set_dpp         = rdr_hw_set_dpp;
        renderer.translate       = rdr_hw_translate;
        renderer.zoom            = rdr_hw_zoom;
        renderer.resize          = rdr_hw_resize;
        renderer.render          = rdr_hw_render;
    }

    /* Init. */
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow(title,
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            width, height,
            SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    if (!window) {
        panic("Error: SDL can't open a window.");
    }
    renderer.init(window, fi[ifi]);

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

        case SDL_WINDOWEVENT:
            switch (event.window.event) {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    SDL_GetWindowSize(window, &width, &height);
                    renderer.resize(width, height);
                    break;
            }
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
                renderer.set_center(fi[ifi].cx, fi[ifi].cy);
                renderer.set_dpp(fi[ifi].dpp);
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
            case SDL_BUTTON_MIDDLE:
                mbpx = event.button.x;
                mbpy = event.button.y;
                update = false;
                break;
            }
            break;

        case SDL_MOUSEBUTTONUP:
            switch(event.button.button) {
            /* Zoom (box). */
            case SDL_BUTTON_LEFT:
                mbrx = event.button.x;
                mbry = event.button.y;

                if(mbrx != mbpx && mbry != mbpy) {
                    /* Center the view... */
                    int center_x = width / 2;
                    int center_y = height / 2;
                    int new_center_x = (mbrx + mbpx) / 2;
                    int new_center_y = (mbry + mbpy) / 2;
                    double tx = ((double)(new_center_x) - center_x) / width;
                    double ty = ((double)(new_center_y) - center_y) / height;
                    renderer.translate(tx, ty);
                    /* ...then zoom in. */
                    double fx = width / (double)abs(mbrx - mbpx);
                    double fy = height / (double)abs(mbry - mbpy);
                    double fc = (fx < fy) ? fx : fy;
                    renderer.zoom(fc);
                }
                break;
            /* Translation. */
            case SDL_BUTTON_MIDDLE:
                mbrx = event.button.x;
                mbry = event.button.y;
                int dx = mbrx - mbpx;
                int dy = mbry - mbpy;
                double tx = (double)(dx) / width;
                double ty = (double)(dy) / height;
                renderer.translate(tx, ty);
            }
            break;

        default:
            update = false;
            break;
        }
    }

    /* Deinit. */
    renderer.free();
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}

