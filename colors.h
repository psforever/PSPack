#ifndef COLORS_H
#define COLORS_H

#include <stdbool.h>

// Define colors by their shorthand AnsiColor case.
extern const char * AC_RED;
extern const char * AC_GREEN;
extern const char * AC_YELLOW;
extern const char * AC_BLUE;
extern const char * AC_MAGENTA;
extern const char * AC_CYAN;
extern const char * AC_RESET;

void colors_enable();
void colors_disable();
bool colors_status();

#endif
