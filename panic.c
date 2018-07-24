#include "panic.h"

#include <stdio.h>
#include <stdlib.h>

void panic(const char* err) {
    fputs(err, stderr);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}
