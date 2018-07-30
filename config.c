#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "vendor/tomlc99/toml.h"

#include "types.h"
#include "panic.h"

static struct fractal_info* config_read_preset(toml_table_t* preset) {
    struct fractal_info* fi = calloc(1, sizeof(struct fractal_info));
    /* Read preset. */
    const char* val = NULL;
    /* key: name */
    if ((val = toml_raw_in(preset, "name"))) {
        //...
    } else {
        //...
    }
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
    if (!((val = toml_raw_in(preset, "dynamic"))
            && toml_rtob(val, (int*)&fi->dynamic) == 0)) {
        fi->dynamic = false;
    }
    /* key: speed */
    if (!((val = toml_raw_in(preset, "speed"))
            && toml_rtod(val, &(fi->speed)) == 0)) {
        fi->speed = 0.0;
    }
    /* table: center */
    toml_table_t* center;
    if ((center = toml_table_in(preset, "center"))) {
        /* key: center.x */
        if (!((val = toml_raw_in(center, "x"))
                && toml_rtod(val, &(fi->cx)) == 0)) {
            fi->cx = 0.0;
        }
        /* key: center.y */
        if (!((val = toml_raw_in(center, "y"))
                && toml_rtod(val, &(fi->cy)) == 0)) {
            fi->cy = 0.0;
        }
    }
    /* key: dpp */
    if (!((val = toml_raw_in(preset, "dpp"))
            && toml_rtod(val, &(fi->dpp)) == 0)) {
        fi->dpp = 0.0;
    }
    /* key: max_iter */
    if (!((val = toml_raw_in(preset, "max_iter"))
            && toml_rtoi(val, (int64_t*)&(fi->max_iter)) == 0)) {
        fi->max_iter = 50;
    }
    /* table: julia */
    toml_table_t* julia;
    if ((julia = toml_table_in(preset, "julia"))) {
        /* key: julia.x */
        if (!((val = toml_raw_in(julia, "x"))
                && toml_rtod(val, &(fi->jx)) == 0)) {
            fi->jx = 0.0;
        }
        /* key: julia.y */
        if (!((val = toml_raw_in(julia, "y"))
                && toml_rtod(val, &(fi->jy)) == 0)) {
            fi->jy = 0.0;
        }
    }
    /* key: n */
    if (!((val = toml_raw_in(preset, "n"))
            && toml_rtoi(val, (int64_t*)&(fi->n)) == 0)) {
        fi->n = 0;
    }
    return fi;
}

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
        panicf("Can't open config file `%s`.", filename);
    }
    /* Parse config file. */
    toml_table_t* conf;
    char errbuf[200];
    conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);
    if (!conf) {
        panicf("Can't parse config file `%s`: %s.", filename, errbuf);
    }
    /* Locate presets array. */
    toml_array_t* presets;
    if (!(presets = toml_array_in(conf, "presets"))) {
        toml_free(conf);
        panicf("Can't find `presets` table in config file `%s`.", filename);
    }
    /* Iterate presets array. */
    toml_table_t* preset;
    for (int i = 0; (preset = toml_table_at(presets, i)); i++) {
        struct fractal_info* fi = NULL;
        if (!(fi = config_read_preset(preset))) {
            toml_free(conf);
            panicf("Can't read preset at index %d in config file `%s`.", i, filename);
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
