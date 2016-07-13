#include "prompt.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "util.h" // for color support

static prompt_choice_t * prompt_find_match(char * choice, prompt_choice_t * choices);
static prompt_choice_t * prompt_find_imatch(char * choice, prompt_choice_t * choices);

int prompt_choice(char * prompt, prompt_choice_t * choices,
    bool sensitive,
    bool keep_going)
{
  do
  {
    char * result = prompt_string(prompt);

    if(!result) {
      return -1;
    }

    prompt_choice_t * c = sensitive ?
      prompt_find_match(result, choices) : prompt_find_imatch(result, choices);

    // match found!
    if(c) {
      free(result);
      return c->val;
    } else {
      // no match and we must keep going. Let the user know they made a bad choice
      // Only show the invalid choice notice when the user actually typed something
      if(keep_going) {
        if(strlen(result))
          printf("%sInvalid choice '%s'%s\n", AC_RED, result, AC_RESET);
      } else {
        free(result);
        return '?';
      }

      free(result);
    }
  } while(keep_going);

  // never reached
  return -1;
}

char * prompt_string(char * prompt)
{
  char * result = NULL;
  size_t result_alloc = 0;
  size_t result_size = 0;

  fprintf(stdout, "%s", prompt);
  fflush(stdout);

  while(true) {
    int chr = fgetc(stdin);

    if(feof(stdin) || ferror(stdin)) {
      free(result);
      return NULL;
    }

    // plus one so we always have room for the null terminator
    if(result_size+1 >= result_alloc) {
      result_alloc += 100;
      result = realloc(result, result_alloc);
    }

    if(chr == '\n')
      break;

    result[result_size++] = chr;
  }

  // null terminate the string
  result[result_size] = '\0';

  return result;
}

static prompt_choice_t * prompt_find_match(char * choice, prompt_choice_t * choices)
{
  prompt_choice_t * c;
  bool matched = false;

  for(c = choices; c->name || c->long_name; c++) {
      if((c->name && strcmp(c->name, choice)) == 0 ||
          (c->long_name && strcmp(c->long_name, choice)) == 0) {
        matched = true;
        break;
      }
  }

  if(matched)
    return c;
  else
    return NULL;
}

static prompt_choice_t * prompt_find_imatch(char * choice, prompt_choice_t * choices)
{
  prompt_choice_t * c;
  bool matched = false;

  for(c = choices; c->name || c->long_name; c++) {
      if((c->name && strcasecmp(c->name, choice)) == 0 ||
          (c->long_name && strcasecmp(c->long_name, choice)) == 0) {
        matched = true;
        break;
      }
  }

  if(matched)
    return c;
  else
    return NULL;
}
