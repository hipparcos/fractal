#ifndef _H_CONFIG_
#define _H_CONFIG_

#include <stdlib.h>

#include "types.h"

struct config {
    /** width of the main window. */
    int width;
    /** height of the main window. */
    int height;
    /** zoom factor (dpp multiplier). */
    double zoomf;
    /** translate factor base on screen size. */
    double translatef;
    /** software is set to 1 if software renderer must be used. */
    int software;
    /** max iteration override. */
    int max_iter;
    /** step to increase/decrease max_iter. */
    int step;
    /** spped multiplier (valid only with dynamic rendering). */
    double speed;
    /** speed_step multiplier to increase/descrease speed. */
    double speed_step;
    /** preset is the index of the selected preset. */
    size_t preset;
    /** presets is a list of preset. */
    struct fractal_info** presets;
    size_t presetc;
};

extern struct config default_config;

/** config_init init cfg. */
void config_init(struct config* cfg);
/** config_read reads the content of filename to cfg.
 ** Caller is responsible for calling config_clear on cfg. */
void config_read(const char* filename, struct config* cfg);
/** config_free frees the content of cfg. */
void config_clear(struct config* cfg);
/** config_fallback set null members of dest to src. */
void config_fallback(struct config* dest, struct config src);

#endif
