#ifndef ANSICOLOR-W32_H
#define ANSICOLOR-W32_H
#ifdef _WIN32

#include <stdio.h>

int _fprintf_w32(FILE* fp, const char* format, ...);

#define fprintf(...) _fprintf_w32(__VA_ARGS__)
#define printf(...) _fprintf_w32(stdout, __VA_ARGS__)
#define fputs(fp, x) _fprintf_w32(fp, x);
#define puts(x) _fprintf_w32(stdout, x);

#endif // _WIN32
#endif // ANSICOLOR-W32_H
