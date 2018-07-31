#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <limits.h>
#include <popt.h>

#include "renderer_software.h"
#include "renderer_hardware.h"
#include "config.h"
#include "panic.h"
#include "types.h"

/* Default config. */
static char* title = "fractal";
struct fractal_info default_fi = {
    .generator = GEN_MANDELBROT,
    .max_iter  = 50,
    .cx        = -0.7,
    .cy        = 0.0,
    .dpp       = 0.0035,
};
struct fractal_info *default_presets[] = {
    &default_fi,
};
struct config default_config = {
    .width      = 800,
    .height     = 600,
    .zoomf      = 1.1,
    .translatef = 0.25,
    .software   = 0,
    .max_iter   = 50,
    .step       = 10,
    .speed      = 1.0,
    .speed_step = 0.33,
    .preset     = 0,
    .presets    = (struct fractal_info**) &default_presets,
    .presetc    = sizeof(default_presets)/sizeof(default_presets[0]),
};

void handle_events(SDL_Window* window, struct renderer* renderer, struct config* cfg,
        struct fractal_info* fi, bool* quit, bool* updt, bool* pause);

int main(int argc, char* argv[]) {
    /* CLI arguments. */
    struct config cli_config = {0};
    struct poptOption optionsTable[] = {
        {"width", 'w', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,
            &cli_config.width, 0, "Set window width", NULL},
        {"height", 'h', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,
            &cli_config.height, 0, "Set window height", NULL},
        {"zoom", 'z', POPT_ARG_DOUBLE|POPT_ARGFLAG_SHOW_DEFAULT,
            &cli_config.zoomf, 0, "Set zoom factor based on screen size", NULL},
        {"translate", 't', POPT_ARG_DOUBLE|POPT_ARGFLAG_SHOW_DEFAULT,
            &cli_config.translatef, 0, "Set translation factor based on screen size", NULL},
        {"iter", 'i', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,
            &cli_config.max_iter, 0, "Set max iteration limit", NULL},
        {"step", '\0', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,
            &cli_config.step, 0, "Set max iteration (incr|decr)ementation step", NULL},
        {"preset", 'p', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,
            &cli_config.preset, 0, "Set fractal preset to use (index of presets, from 0)", NULL},
        {"speed", '\0', POPT_ARG_DOUBLE|POPT_ARGFLAG_SHOW_DEFAULT,
            &cli_config.speed, 0, "Set dynamic fractals rendering speed", NULL},
        {"software", 's', POPT_ARG_INT|POPT_ARGFLAG_SHOW_DEFAULT,
            &cli_config.software, 0, "Use software renderer (hardware renderer by default)", "0|1"},
        POPT_AUTOHELP
        POPT_TABLEEND
    };

    poptContext optCon = poptGetContext(NULL, argc, (const char**) argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]");

    /* Custom CLI actions. */
    char c;
    while ((c = poptGetNextOpt(optCon)) >= 0) {
        switch (c) {
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

    /* Read config from config file. */
    struct config cfg = {0};
    config_init(&cfg);
    config_read("config.toml", &cfg);

    /* Override config by cli_config, default to default_config. */
    config_fallback(&cfg, default_config);
    config_override(&cfg, cli_config);

    /* Select renderer. */
    struct renderer renderer;
    if (cfg.software) {
        renderer = sw_renderer;
    } else {
        renderer = hw_renderer;
    }

    /* Init. */
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow(title,
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            cfg.width, cfg.height,
            SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    if (!window) {
        panic("Error: SDL can't open a window.");
    }
    renderer.init(window);

    struct fractal_info fi = *(cfg.presets[cfg.preset]);

    /* Main loop variables. */
    bool quit = false;
    bool updt = true;
    bool pause = false;
    double t  = 0.0;
    double dt = 0.0;
    uint32_t old_time = SDL_GetTicks();
    uint32_t min_frame_time = 1000/60; // 60 fps limit.
    uint32_t frame = 0;
    /* FPS display */
    uint32_t last_fps_display_time = old_time;
    uint32_t last_fps_display_at_frame = 0;
    uint32_t fps_display_interval = 1000; // 1 / s.

    /* Main loop. */
    while (!quit) {
        /* Event handling */
        handle_events(window, &renderer, &cfg, &fi, &quit, &updt, &pause);

        /* Rendering */
        if (updt) {
            renderer.render(fi, t, dt);
            if (!fi.dynamic) {
                updt = false;
            }
        }

        /* FPS limiter */
        uint32_t new_time = SDL_GetTicks();
        uint32_t frame_time = new_time - old_time;
        old_time = new_time;
        if (frame_time < min_frame_time) {
            SDL_Delay(min_frame_time - frame_time);
        }
        if (fi.dynamic && !pause) {
            dt = 0.001 * (double)frame_time;
            t += dt * fi.speed;
        }
        frame++;

        /* Display fps in console. */
        if (fi.dynamic && new_time > last_fps_display_time + fps_display_interval) {
            fprintf(stdout, "> %d frames per second\n", frame - last_fps_display_at_frame);
            last_fps_display_time = new_time;
            last_fps_display_at_frame = frame;
        } else if (!fi.dynamic) {
            last_fps_display_time = new_time;
            last_fps_display_at_frame = frame;
        }
    }

    /* Deinit. */
    renderer.free();
    SDL_DestroyWindow(window);
    SDL_Quit();
    config_clear(&cfg);

    return EXIT_SUCCESS;
}

/** handle_events responds to SDL events. Depends on config variables. */
void handle_events(SDL_Window* window, struct renderer* renderer, struct config* cfg,
        struct fractal_info* fi, bool* quit, bool* updt, bool* pause) {
    SDL_Event event;
    static Uint16 mbpx = 0, mbpy = 0, mbrx = 0, mbry = 0;
    int width, height;
    /* Events */
    while (SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_QUIT:
                *quit = true;
                break;

            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        SDL_GetWindowSize(window, &width, &height);
                        renderer->resize(width, height);
                        *updt = true;
                        break;
                }
                break;

            case SDL_KEYDOWN:
                switch(event.key.keysym.sym) {
                    case SDLK_q:
                    case SDLK_ESCAPE:
                        *quit = true;
                        break;

                    case SDLK_u:
                        *updt = true;
                        *pause = false;
                        break;

                    case SDLK_SPACE:
                        *updt = true;
                        *pause = !(*pause);
                        break;

                    case SDLK_UP:
                        fi_translate(fi, window, 0,  cfg->translatef);
                        *updt = true;
                        break;
                    case SDLK_DOWN:
                        fi_translate(fi, window, 0, -cfg->translatef);
                        *updt = true;
                        break;
                    case SDLK_RIGHT:
                        fi_translate(fi, window,  cfg->translatef, 0);
                        *updt = true;
                        break;
                    case SDLK_LEFT:
                        fi_translate(fi, window, -cfg->translatef, 0);
                        *updt = true;
                        break;

                    case SDLK_p:
                    case SDLK_PLUS:
                    case SDLK_KP_PLUS:
                        if((event.key.keysym.mod & KMOD_LCTRL) == KMOD_LCTRL ||
                                (event.key.keysym.mod & KMOD_RCTRL) == KMOD_RCTRL) {
                            fi_zoom(fi, cfg->zoomf);
                        } else {
                            fi_max_iter_incr(fi, cfg->step);
                        }
                        *updt = true;
                        break;
                    case SDLK_m:
                    case SDLK_MINUS:
                    case SDLK_KP_MINUS:
                        if((event.key.keysym.mod & KMOD_LCTRL) == KMOD_LCTRL ||
                                (event.key.keysym.mod & KMOD_RCTRL) == KMOD_RCTRL) {
                            fi_zoom(fi, 1/cfg->zoomf);
                        } else {
                            fi_max_iter_decr(fi, cfg->step);
                        }
                        *updt = true;
                        break;

                    case SDLK_a:
                        fi->speed += cfg->speed_step;
                        *updt = true;
                        *pause = false;
                        break;
                    case SDLK_d:
                        fi->speed -= cfg->speed_step;
                        *updt = true;
                        *pause = false;
                        break;

                        /* Switch. */
                    case SDLK_s:
                        cfg->preset += 1;
                        cfg->preset %= cfg->presetc;
                        *pause = false;
                        /* Reset. */
                    case SDLK_r:
                        *fi = *cfg->presets[cfg->preset];
                        *updt = true;
                        break;
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                switch(event.button.button) {
                    case SDL_BUTTON_LEFT:
                    case SDL_BUTTON_MIDDLE:
                        mbpx = event.button.x;
                        mbpy = event.button.y;
                        *updt = false;
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
                            fi_translate(fi, window, tx, -ty);
                            /* ...then zoom in. */
                            double fx = width / (double)abs(mbrx - mbpx);
                            double fy = height / (double)abs(mbry - mbpy);
                            double fc = (fx < fy) ? fx : fy;
                            fi_zoom(fi, fc);
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
                        fi_translate(fi, window, tx, -ty);
                        break;
                }
                *updt = true;
                break;
        }
    }
}
