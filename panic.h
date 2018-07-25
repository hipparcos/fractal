#ifndef _H_PANIC_
#define _H_PANIC_

/** panic displays an error and exits with the status EXIT_FAILURE. */
void panic(const char* err);
/** panicf displays an error and exits with the status EXIT_FAILURE.
 ** format and list of arguments are passed to vfprintf as is. */
void panicf(const char* format, ...);

#endif
