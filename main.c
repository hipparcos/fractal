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
static char* config_file = "config.toml";
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
    .iter_step  = 10,
    .speed      = 1.0,
    .speed_step = 0.33,
    .preset     = 0,
    .presets    = (struct fractal_info**) &default_presets,
    .presetc    = sizeof(default_presets)/sizeof(default_presets[0]),
};

struct state {
    SDL_Window*          window;
    struct renderer*     renderer;
    struct config*       cfg;
    struct fractal_info  fi;
    bool quit;
    bool updt;
    bool pause;
    double t;
    double dt;
};

void handle_events(struct state* state);

int main(int argc, char* argv[]) {
    /* CLI arguments. */
    struct config cli_config = {0};
    struct poptOption optionsTable[] = {
        {"config", 'c', POPT_ARG_STRING|POPT_ARGFLAG_SHOW_DEFAULT,
            &config_file, 0, "Set config file", ""},
        {"width", 'w', POPT_ARG_INT,
            &cli_config.width, 0, "Set window width in pixels", NULL},
        {"height", 'h', POPT_ARG_INT,
            &cli_config.height, 0, "Set window height in pixels", NULL},
        {"zoom", 'z', POPT_ARG_DOUBLE,
            &cli_config.zoomf, 0, "Set zoom factor (density per pixel multiplier)", NULL},
        {"translate", 't', POPT_ARG_DOUBLE,
            &cli_config.translatef, 0, "Set translation factor (screen size multiplier)", NULL},
        {"iter", 'i', POPT_ARG_INT,
            &cli_config.max_iter, 0, "Set max iteration limit", NULL},
        {"step", '\0', POPT_ARG_INT,
            &cli_config.iter_step, 0, "Set max iteration (incr|decr)ementation step", NULL},
        {"preset", 'p', POPT_ARG_INT,
            &cli_config.preset, 0, "Set fractal preset to use (index of presets, from 0)", NULL},
        {"speed", '\0', POPT_ARG_DOUBLE,
            &cli_config.speed, 0, "Set dynamic fractals rendering speed", NULL},
        {"software", 's', POPT_ARG_INT,
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
    config_read(config_file, &cfg);

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

    /* Main loop variables. */
    struct state state = {
        .window=   window,
        .renderer= &renderer,
        .cfg=      &cfg,
        .fi=       *(cfg.presets[cfg.preset]),
        .quit=  false,
        .updt=  true,
        .pause= false,
        .t  = 0.0,
        .dt = 0.0,
    };
    uint32_t old_time = SDL_GetTicks();
    uint32_t min_frame_time = 1000/60; // 60 fps limit.
    uint32_t frame = 0;
    /* FPS display */
    uint32_t last_fps_display_time = old_time;
    uint32_t last_fps_display_at_frame = 0;
    uint32_t fps_display_interval = 1000; // 1 / s.

    /* Main loop. */
    while (!state.quit) {
        /* Event handling */
        handle_events(&state);

        /* Rendering */
        if (state.updt) {
            renderer.render(state.fi, state.t, state.dt);
            if (!state.fi.dynamic) {
                state.updt = false;
            }
        }

        /* FPS limiter */
        uint32_t new_time = SDL_GetTicks();
        uint32_t frame_time = new_time - old_time;
        old_time = new_time;
        if (frame_time < min_frame_time) {
            SDL_Delay(min_frame_time - frame_time);
        }
        if (state.fi.dynamic && !state.pause) {
            state.dt = 0.001 * (double)frame_time;
            state.t += state.dt * state.fi.speed;
        }
        frame++;

        /* Display fps in console. */
        if (state.fi.dynamic && new_time > last_fps_display_time + fps_display_interval) {
            fprintf(stdout, "> %d frames per second\n", frame - last_fps_display_at_frame);
            last_fps_display_time = new_time;
            last_fps_display_at_frame = frame;
        } else if (!state.fi.dynamic) {
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
void handle_events(struct state* state) {
    SDL_Event event;
    static Uint16 mbpx = 0, mbpy = 0, mbrx = 0, mbry = 0;
    int width, height;
    SDL_GetWindowSize(state->window, &width, &height);
    /* Events */
    while (SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_QUIT:
                state->quit = true;
                break;

            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        SDL_GetWindowSize(state->window, &width, &height);
                        state->renderer->resize(width, height);
                        state->updt = true;
                        break;
                }
                break;

            case SDL_KEYDOWN:
                switch(event.key.keysym.sym) {
                    case SDLK_q:
                    case SDLK_ESCAPE:
                        state->quit = true;
                        break;

                    case SDLK_u:
                        state->updt = true;
                        state->pause = false;
                        break;

                    case SDLK_SPACE:
                        state->updt = true;
                        state->pause = !(state->pause);
                        break;

                    case SDLK_UP:
                        fi_translate(&state->fi, state->window, 0,  state->cfg->translatef);
                        state->updt = true;
                        break;
                    case SDLK_DOWN:
                        fi_translate(&state->fi, state->window, 0, -state->cfg->translatef);
                        state->updt = true;
                        break;
                    case SDLK_RIGHT:
                        fi_translate(&state->fi, state->window,  state->cfg->translatef, 0);
                        state->updt = true;
                        break;
                    case SDLK_LEFT:
                        fi_translate(&state->fi, state->window, -state->cfg->translatef, 0);
                        state->updt = true;
                        break;

                    case SDLK_p:
                    case SDLK_PLUS:
                    case SDLK_KP_PLUS:
                        if((event.key.keysym.mod & KMOD_LCTRL) == KMOD_LCTRL ||
                                (event.key.keysym.mod & KMOD_RCTRL) == KMOD_RCTRL) {
                            fi_zoom(&state->fi, state->cfg->zoomf);
                        } else {
                            fi_max_iter_incr(&state->fi, state->cfg->iter_step);
                        }
                        state->updt = true;
                        break;
                    case SDLK_m:
                    case SDLK_MINUS:
                    case SDLK_KP_MINUS:
                        if((event.key.keysym.mod & KMOD_LCTRL) == KMOD_LCTRL ||
                                (event.key.keysym.mod & KMOD_RCTRL) == KMOD_RCTRL) {
                            fi_zoom(&state->fi, 1/state->cfg->zoomf);
                        } else {
                            fi_max_iter_decr(&state->fi, state->cfg->iter_step);
                        }
                        state->updt = true;
                        break;

                    case SDLK_a:
                        state->fi.speed += state->cfg->speed_step;
                        state->updt = true;
                        state->pause = false;
                        break;
                    case SDLK_d:
                        state->fi.speed -= state->cfg->speed_step;
                        state->updt = true;
                        state->pause = false;
                        break;

                        /* Switch. */
                    case SDLK_s:
                        state->cfg->preset += 1;
                        state->cfg->preset %= state->cfg->presetc;
                        state->pause = false;
                        /* Reset. */
                    case SDLK_r:
                        state->fi = *(state->cfg->presets[state->cfg->preset]);
                        state->t  = 0.0;
                        state->dt = 0.0;
                        state->updt = true;
                        break;
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                switch(event.button.button) {
                    case SDL_BUTTON_LEFT:
                    case SDL_BUTTON_MIDDLE:
                        mbpx = event.button.x;
                        mbpy = event.button.y;
                        state->updt = false;
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
                            fi_translate(&state->fi, state->window, tx, -ty);
                            /* ...then zoom in. */
                            double fx = width / (double)abs(mbrx - mbpx);
                            double fy = height / (double)abs(mbry - mbpy);
                            double fc = (fx < fy) ? fx : fy;
                            fi_zoom(&state->fi, fc);
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
                        fi_translate(&state->fi, state->window, tx, -ty);
                        break;
                }
                state->updt = true;
                break;
        }
    }
}
