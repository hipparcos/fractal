#ifndef _H_CONFIG_
#define _H_CONFIG_

#include <stdlib.h>

#include "types.h"

struct config {
    struct fractal_info** presets;
    size_t presetc;
};

/** config_init init cfg. */
void config_init(struct config* cfg);
/** config_read reads the content of filename to cfg.
 ** Caller is responsible for calling config_clear on cfg. */
void config_read(const char* filename, struct config* cfg);
/** config_free frees the content of cfg. */
void config_clear(struct config* cfg);

#endif
