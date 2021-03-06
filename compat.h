#ifndef FLATOOL_COMPAT_H
#define FLATOOL_COMPAT_H

#if defined(_WIN32) || defined(__CYGWIN__)
#define PLATFORM_WINDOWS
#elif defined(__linux__)
#define PLATFORM_LINUX
#define PLATFORM_UNIX // Linux is Unix
#elif defined(__APPLE__)
#define PLATFORM_APPLE
#define PLATFORM_UNIX // Unix isnt Linux
#else
#error "unsupported platform"
#endif

#if defined(__x86_64__) || defined(__amd64__)
#define PLATFORM_BITS_64
#elif defined(__i386__) || defined(_X86_)
#define PLATFORM_BITS_32
#else
#error "unsupported bit width"
#endif

#endif
