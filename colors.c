#include "colors.h"

// Define colors by their shorthand AnsiColor case.
// Local copy
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define RESET "\x1b[0m"

// Dynamic strings
const char * AC_RED = RED;
const char * AC_GREEN = GREEN;
const char * AC_YELLOW = YELLOW;
const char * AC_BLUE = BLUE;
const char * AC_MAGENTA = MAGENTA;
const char * AC_CYAN = CYAN;
const char * AC_RESET = RESET;

bool colors_enabled = true;

void colors_enable()
{
  if(colors_enabled)
    return;

  AC_RED = RED;
  AC_GREEN = GREEN;
  AC_YELLOW = YELLOW;
  AC_BLUE = BLUE;
  AC_MAGENTA = MAGENTA;
  AC_CYAN = CYAN;
  AC_RESET = RESET;

  colors_enabled = true;
}

void colors_disable()
{
  if(!colors_enabled)
    return;

  AC_RED = "";
  AC_GREEN = "";
  AC_YELLOW = "";
  AC_BLUE = "";
  AC_MAGENTA = "";
  AC_CYAN = "";
  AC_RESET = "";

  colors_enabled = false;
}

bool colors_status()
{
  return colors_enabled;
}
