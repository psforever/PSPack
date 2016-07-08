/*
Copyright (C) 2014 insane coder (http://insanecoding.blogspot.com/, http://asprintf.insanecoding.org/)

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "compat.h"

#include "asprintf.h"

#define insane_free(ptr) { free(ptr); ptr = 0; }

int asprintf(char **strp, const char *fmt, ...)
{
  int r;
  va_list ap;
  va_start(ap, fmt);
  r = vasprintf(strp, fmt, ap);
  va_end(ap);
  return(r);
}

#ifndef PLATFORM_WINDOWS
static int _vscprintf(const char * format, va_list pargs)
{ 
  int retval; 
  va_list argcopy;
  va_copy(argcopy, pargs); 
  retval = vsnprintf(NULL, 0, format, argcopy); 
  va_end(argcopy); 
  return retval;
}
#endif

int vasprintf(char **strp, const char *fmt, va_list ap)
{
  int r = -1, size = _vscprintf(fmt, ap);
  if ((size >= 0) && (size < INT_MAX))
  {
    *strp = (char *)malloc(size+1); //+1 for null
    if (*strp)
    {
      r = vsnprintf(*strp, size+1, fmt, ap);  //+1 for null
      if (r == -1) { insane_free(*strp); }
    }
  }
  else { *strp = 0; }
  return(r);
}
