#ifndef _H_PANIC_
#define _H_PANIC_

#include <stdio.h>
#include <errno.h>

/** panicen displays msg followed by the errno error message. */
#define panicen(en, msg) \
    do { \
        errno = en; \
        fprintf(stderr, "%s:%d:%s:", __FILE__, __LINE__, __func__); \
        perror(msg); exit(EXIT_FAILURE); \
    } while (0)

/** panic displays an error and exits with the status EXIT_FAILURE. */
void panic(const char* err);
/** panicf displays an error and exits with the status EXIT_FAILURE.
 ** format and list of arguments are passed to vfprintf as is. */
void panicf(const char* format, ...);

#endif
