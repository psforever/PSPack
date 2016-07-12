#ifndef ANSICOLOR_W32_H
#define ANSICOLOR_W32_H
#ifdef _WIN32

#include <stdio.h>

int _fprintf_w32(FILE* fp, const char* format, ...);

#define fprintf(...) do { \
  if(!is_cygwin()) \
    _fprintf_w32(__VA_ARGS__); \
  else \
    fprintf(__VA_ARGS__); \
} while(0)
#define printf(...) do { \
  if(!is_cygwin()) \
    _fprintf_w32(stdout, __VA_ARGS__); \
  else \
    printf(__VA_ARGS__); \
} while(0)
#define fputs(fp, x) do { \
  if(!is_cygwin()) \
    _fprintf_w32(fp, x); \
  else \
    fputs(fp, x); \
} while(0)
#define puts(x) do { \
  if(!is_cygwin()) \
    _fprintf_w32(stdout, x); \
  else \
    puts(x); \
} while(0)

#endif // _WIN32
#endif // ANSICOLOR_W32_H
