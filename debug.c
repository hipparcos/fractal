#include "debug.h"

#include <stdio.h>

void debug(const char* format, ...)
{
#ifdef DEBUG
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  printf("\n");
#endif
}

void debug_separator(void)
{
#ifdef DEBUG
  puts("-----");
#endif
}
