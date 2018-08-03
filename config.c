#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "vendor/tomlc99/toml.h"

#include "types.h"
#include "panic.h"

static void read_bool(toml_table_t* conf, const char* key, bool* dest, bool fallback) {
    const char* val = NULL;
    if (!((val = toml_raw_in(conf, key))
            && toml_rtob(val, (int*)dest) == 0)) {
        *dest = fallback;
    }
}
static void read_int(toml_table_t* conf, const char* key, int* dest, int fallback) {
    const char* val = NULL;
    if (!((val = toml_raw_in(conf, key))
            && toml_rtoi(val, (int64_t*)dest) == 0)) {
        *dest = fallback;
    }
}
static void read_double(toml_table_t* conf, const char* key, double* dest, double fallback) {
    const char* val = NULL;
    if (!((val = toml_raw_in(conf, key))
            && toml_rtod(val, dest) == 0)) {
        *dest = fallback;
    }
}

static struct fractal_info* config_read_preset(toml_table_t* preset) {
    struct fractal_info* fi = calloc(1, sizeof(struct fractal_info));
    /* Read preset. */
    const char* val = NULL;
    /* key: generator */
    if ((val = toml_raw_in(preset, "generator"))) {
        char* gen = NULL;
        toml_rtos(val, &gen);
        if      (strcmp(gen, "julia") == 0)
            fi->generator = GEN_JULIA;
        else if (strcmp(gen, "julia_multiset") == 0)
            fi->generator = GEN_JULIA_MULTISET;
        else
            fi->generator = GEN_MANDELBROT;
        free(gen);
    } else {
        fi->generator = GEN_MANDELBROT;
    }
    /* key: dynamic */
    read_bool(preset, "dynamic", &(fi->dynamic), false);
    /* key: speed */
    read_double(preset, "speed", &(fi->speed), 0.0);
    /* table: center */
    toml_table_t* center;
    if ((center = toml_table_in(preset, "center"))) {
        /* key: center.x */
        read_double(center, "x", &(fi->cx), 0.0);
        /* key: center.y */
        read_double(center, "y", &(fi->cy), 0.0);
    }
    /* key: dpp */
    read_double(preset, "dpp", &(fi->dpp), 0.0);
    /* key: max_iter */
    read_int(preset, "max_iter", &(fi->max_iter), 0);
    /* table: julia */
    toml_table_t* julia;
    if ((julia = toml_table_in(preset, "julia"))) {
        /* key: julia.x */
        read_double(julia, "x", &(fi->jx), 0.0);
        /* key: julia.y */
        read_double(julia, "y", &(fi->jy), 0.0);
    }
    /* key: n */
    read_int(preset, "n", &(fi->n), 0);
    return fi;
}

static void config_read_base(toml_table_t* conf, struct config* cfg) {
    read_int(conf,    "width",      &(cfg->width),        0);
    read_int(conf,    "height",     &(cfg->height),       0);
    read_double(conf, "zoomf",      &(cfg->zoomf),        0.0);
    read_double(conf, "translatef", &(cfg->translatef),   0.0);
    read_int(conf,    "software",   &(cfg->software),     0);
    read_int(conf,    "iter_step",  &(cfg->iter_step),    0.0);
    read_double(conf, "speed_step", &(cfg->speed_step),   0.0);
    read_int(conf,    "preset",     (int*)&(cfg->preset), 0);
};

void config_init(struct config* cfg) {
    if (!cfg) {
        return;
    }
    cfg->presetc = 0;
    cfg->presets = NULL;
}

void config_read(const char* filename, struct config* cfg) {
    /* Read config file. */
    FILE* fp;
    if (!(fp = fopen(filename, "r"))) {
        fprintf(stderr, "Can't open config file `%s`.\n", filename);
        return;
    }
    /* Parse config file. */
    toml_table_t* conf;
    char errbuf[200];
    conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);
    if (!conf) {
        fprintf(stderr, "Can't parse config file `%s`: %s.\n", filename, errbuf);
        return;
    }
    /* Read general config infos. */
    config_read_base(conf, cfg);
    /* Locate presets array. */
    toml_array_t* presets;
    if (!(presets = toml_array_in(conf, "presets"))) {
        toml_free(conf);
        fprintf(stderr, "Can't find `presets` table in config file `%s`.\n", filename);
        return;
    }
    /* Iterate presets array. */
    toml_table_t* preset;
    for (int i = 0; (preset = toml_table_at(presets, i)); i++) {
        struct fractal_info* fi = NULL;
        if (!(fi = config_read_preset(preset))) {
            toml_free(conf);
            fprintf(stderr, "Can't read preset at index %d in config file `%s`.\n", i, filename);
            return;
        }
        /* Append fi to cfg->presets. */
        cfg->presetc++;
        cfg->presets = realloc(cfg->presets, cfg->presetc * sizeof(struct fractal_info*));
        cfg->presets[cfg->presetc - 1] = fi;
    }
    toml_free(conf);
}

void config_clear(struct config* cfg) {
    if (!cfg) {
        return;
    }
    for (size_t i = 0; i < cfg->presetc; i++) {
        if (cfg->presets[i]) {
            free(cfg->presets[i]);
        }
    }
    if (cfg->presets) {
        free(cfg->presets);
    }
}

#define FB_IF_NOT_SET_IN_dest(property, nil) if (dest->property == nil) { dest->property = src.property; }
void config_fallback(struct config* dest, struct config src) {
    if (!dest) {
        return;
    }

    FB_IF_NOT_SET_IN_dest(width,      0);
    FB_IF_NOT_SET_IN_dest(height,     0);
    FB_IF_NOT_SET_IN_dest(zoomf,      0.0);
    FB_IF_NOT_SET_IN_dest(translatef, 0.0);
    FB_IF_NOT_SET_IN_dest(software,   0);
    FB_IF_NOT_SET_IN_dest(max_iter,   0);
    FB_IF_NOT_SET_IN_dest(iter_step,  0);
    FB_IF_NOT_SET_IN_dest(speed,      0.0);
    FB_IF_NOT_SET_IN_dest(speed_step, 0.0);

    if (dest->presetc == 0) {
        /* Copy presets. */
        dest->presets = calloc(src.presetc, sizeof(struct fractal_info*));
        for (size_t i = 0; i < src.presetc; i++) {
            dest->presets[i] = malloc(sizeof(struct fractal_info));
            *(dest->presets[i]) = *(src.presets[i]);
        }
        dest->presetc = src.presetc;
        dest->preset = src.preset;
    }
    if (dest->preset >= dest->presetc) {
        dest->preset = 0;
    }
}

#define OR_IF_SET_IN_src(property, nil) if (src.property != nil) { dest->property = src.property; }
void config_override(struct config* dest, struct config src) {
    if (!dest) {
        return;
    }

    OR_IF_SET_IN_src(width,      0);
    OR_IF_SET_IN_src(height,     0);
    OR_IF_SET_IN_src(zoomf,      0.0);
    OR_IF_SET_IN_src(translatef, 0.0);
    OR_IF_SET_IN_src(software,   0);
    OR_IF_SET_IN_src(max_iter,   0);
    OR_IF_SET_IN_src(iter_step,  0);
    OR_IF_SET_IN_src(speed,      0.0);
    OR_IF_SET_IN_src(speed_step, 0.0);
    OR_IF_SET_IN_src(preset,     0);

    /* Propagate max_iter & speed to presets. */
    if (src.max_iter != 0) {
        for(size_t i = 0; i < dest->presetc; i++) {
            dest->presets[i]->max_iter = src.max_iter;
        }
    }
    if (src.speed != 0) {
        for(size_t i = 0; i < dest->presetc; i++) {
            dest->presets[i]->speed = src.speed;
        }
    }
    if (dest->preset >= dest->presetc) {
        dest->preset = 0;
    }
}
