#ifndef PROMPT_H
#define PROMPT_H

#include <stdbool.h>

typedef struct prompt_choice_
{
  const char * name;
  const char * long_name;
  int val;
} prompt_choice_t;

int prompt_choice(char * prompt, prompt_choice_t * choices,
    bool sensitive,
    bool keep_going);
char * prompt_string(char * prompt);

#endif
