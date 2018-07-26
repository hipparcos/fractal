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
static char*  title      = "fractal";
static int    width      = 800;
static int    height     = 600;
static double zoomf      = 1.1;
static double translatef = 0.25;
static int    software   = 1;
static int    max_iter   = 50;
static enum generator generator = GEN_MANDELBROT;

void max_iter_incr(struct fractal_info* fi, int step) {
    if (fi->max_iter > INT_MAX - step) {
        fi->max_iter = INT_MAX;
    } else {
        fi->max_iter += step;
    }
}

void max_iter_decr(struct fractal_info* fi, int step) {
    if (fi->max_iter < step) {
        fi->max_iter = 0;
    } else {
        fi->max_iter -= step;
    }
}

void translate(struct fractal_info* fi, SDL_Window* window, double dx, double dy) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    fi->cx += dx * width * fi->dpp;
    fi->cy -= dy * height * fi->dpp;
}

void zoom(struct fractal_info* fi, double factor) {
    if (factor < 0.001) {
        return;
    }
    fi->dpp *= 1/factor;
}

#define LENGTH(arr) sizeof(arr)/sizeof(arr[0])

int main(int argc, char* argv[]) {
    /* Default config. */
    struct fractal_info fractals[] = {
        {
            .generator= GEN_MANDELBROT,
            .cx=  -0.7,
            .cy=   0.0,
            .dpp=  0.0035,
            .max_iter = max_iter,
        },
        {
            .generator= GEN_JULIA,
            .cx=   0.0,
            .cy=   0.0,
            .dpp=  0.00425,
            .max_iter = max_iter,
        },
    };

    /* CLI arguments. */
    struct poptOption optionsTable[] = {
        {"width", 'w', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,
            &width, 0, "Set window width", NULL},
        {"height", 'h', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,
            &height, 0, "Set window height", NULL},
        {"zoom", 'z', POPT_ARG_DOUBLE|POPT_ARGFLAG_SHOW_DEFAULT,
            &zoomf, 0, "Set zoom factor based on screen size", NULL},
        {"translate", 't', POPT_ARG_DOUBLE|POPT_ARGFLAG_SHOW_DEFAULT,
            &translatef, 0, "Set translation factor based on screen size", NULL},
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
                    generator = GEN_JULIA;
                } else {
                    generator = GEN_MANDELBROT;
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

    for (size_t i = 0; i < LENGTH(fractals); i++) {
        fractals[i].max_iter = max_iter;
    }

    /* Select renderer. */
    struct renderer renderer;
    if (software) {
        renderer = sw_renderer;
    } else {
        renderer = hw_renderer;
    }
    /* Set fractal info. */
    struct fractal_info fractal = fractals[generator];

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
    renderer.init(window);

    /* Main loop. */
    bool quit = false;
    bool update = true;
    SDL_Event event;
    Uint16 mbpx = 0, mbpy = 0, mbrx = 0, mbry = 0;

    while(!quit) {
        /* Display */
        if (update) {
            renderer.render(fractal);
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
                translate(&fractal, window, 0,  translatef);
                break;
            case SDLK_DOWN:
                translate(&fractal, window, 0, -translatef);
                break;
            case SDLK_RIGHT:
                translate(&fractal, window,  translatef, 0);
                break;
            case SDLK_LEFT:
                translate(&fractal, window, -translatef, 0);
                break;

            case SDLK_p:
            case SDLK_PLUS:
            case SDLK_KP_PLUS:
                if((event.key.keysym.mod & KMOD_LCTRL) == KMOD_LCTRL ||
                        (event.key.keysym.mod & KMOD_RCTRL) == KMOD_RCTRL) {
                    zoom(&fractal, zoomf);
                } else {
                    max_iter_incr(&fractal, 10);
                }
                break;
            case SDLK_m:
            case SDLK_MINUS:
            case SDLK_KP_MINUS:
                if((event.key.keysym.mod & KMOD_LCTRL) == KMOD_LCTRL ||
                        (event.key.keysym.mod & KMOD_RCTRL) == KMOD_RCTRL) {
                    zoom(&fractal, 1/zoomf);
                } else {
                    max_iter_decr(&fractal, 10);
                }
                break;

            /* Switch. */
            case SDLK_s:
                generator += 1;
                generator %= LENGTH(fractals);
            /* Reset. */
            case SDLK_r:
                fractal = fractals[generator];
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
                    translate(&fractal, window, tx, -ty);
                    /* ...then zoom in. */
                    double fx = width / (double)abs(mbrx - mbpx);
                    double fy = height / (double)abs(mbry - mbpy);
                    double fc = (fx < fy) ? fx : fy;
                    zoom(&fractal, fc);
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
                translate(&fractal, window, tx, -ty);
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

