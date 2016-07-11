#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <inttypes.h>

#include "compat.h"

#ifdef PLATFORM_WINDOWS
#include "ansicolor-w32.h"
#endif

#include "colors.h"

void fatal(char * msg, ...);
char * basename(const char * path, bool extension);
char * string_cat(const char * l, const char * r);
char * get_extension(char * path);

#define min(x, y) (((x) < (y)) ? (x) : (y))
#define max(x, y) (((x) < (y)) ? (y) : (x))

#ifdef PLATFORM_WINDOWS

#ifdef PLATFORM_BITS_64
#define PRIuSZT "%lu"
#define PRIxSZT "%lx"
#else
#define PRIuSZT "%u"
#define PRIxSZT "%x"
#endif // PLATFORM_BITS_64

#else

#define PRIuSZT "%zu"
#define PRIxSZT "%zx"

#endif // PLATFORM_WINDOWS

#endif // UTIL_H
