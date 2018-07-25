#include "panic.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void panic(const char* err) {
    fputs(err, stderr);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}

void panicf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}
