/*
 * Copyright (c) 2006, Adam Dunkels
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#define DEBUG 0

#if DEBUG
#define DEBUG_PRINTF(...)  printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif

#include "ubasic_tokenizer.h"
#include <ytypes.h>
#include <yassert.h>
#include <ystring.h>

static char const *ptr, *nextptr;

#define MAX_NUMLEN 6

struct keyword_token {
  fstring_id keyword;
  int16_t token;
};

static int16_t current_token = TOKENIZER_ERROR;

static const struct keyword_token keywords[] = {
  { UBASIC_TOKENIZER_LET, TOKENIZER_LET},
  { UBASIC_TOKENIZER_PRINT, TOKENIZER_PRINT},
  { UBASIC_TOKENIZER_IF, TOKENIZER_IF},
  { UBASIC_TOKENIZER_THEN, TOKENIZER_THEN},
  { UBASIC_TOKENIZER_ELSE, TOKENIZER_ELSE},
  { UBASIC_TOKENIZER_FOR, TOKENIZER_FOR},
  { UBASIC_TOKENIZER_TO, TOKENIZER_TO},
  { UBASIC_TOKENIZER_NEXT, TOKENIZER_NEXT},
  { UBASIC_TOKENIZER_GOTO, TOKENIZER_GOTO},
  { UBASIC_TOKENIZER_GOSUB, TOKENIZER_GOSUB},
  { UBASIC_TOKENIZER_RETURN, TOKENIZER_RETURN},
  { UBASIC_TOKENIZER_CALL, TOKENIZER_CALL},
  { UBASIC_TOKENIZER_REM, TOKENIZER_REM},
  { UBASIC_TOKENIZER_PEEK, TOKENIZER_PEEK},
  { UBASIC_TOKENIZER_POKE, TOKENIZER_POKE},
  { UBASIC_TOKENIZER_END, TOKENIZER_END},
  { UBASIC_TOKENIZER_ERROR, TOKENIZER_ERROR}
};

/*---------------------------------------------------------------------------*/
static int
singlechar(void)
{
  if(*ptr == '\n') {
    return TOKENIZER_CR;
  } else if(*ptr == ',') {
    return TOKENIZER_COMMA;
  } else if(*ptr == ';') {
    return TOKENIZER_SEMICOLON;
  } else if(*ptr == '+') {
    return TOKENIZER_PLUS;
  } else if(*ptr == '-') {
    return TOKENIZER_MINUS;
  } else if(*ptr == '&') {
    return TOKENIZER_AND;
  } else if(*ptr == '|') {
    return TOKENIZER_OR;
  } else if(*ptr == '*') {
    return TOKENIZER_ASTR;
  } else if(*ptr == '/') {
    return TOKENIZER_SLASH;
  } else if(*ptr == '%') {
    return TOKENIZER_MOD;
  } else if(*ptr == '(') {
    return TOKENIZER_LEFTPAREN;
  } else if(*ptr == '#') {
    return TOKENIZER_HASH;
  } else if(*ptr == ')') {
    return TOKENIZER_RIGHTPAREN;
  } else if(*ptr == '<') {
    return TOKENIZER_LT;
  } else if(*ptr == '>') {
    return TOKENIZER_GT;
  } else if(*ptr == '=') {
    return TOKENIZER_EQ;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int16_t
get_next_token(void)
{
  struct keyword_token const *kt;
  int16_t i;

  DEBUG_PRINTF("get_next_token(): '%s'\n", ptr);

  if(*ptr == 0) {
    return TOKENIZER_ENDOFINPUT;
  }

  if(isdigit(*ptr)) {
    for(i = 0; i < MAX_NUMLEN; ++i) {
      if(!isdigit(ptr[i])) {
        if(i > 0) {
          nextptr = ptr + i;
          return TOKENIZER_NUMBER;
        } else {
          DEBUG_PRINTF("get_next_token: error due to too short number\n");
          return TOKENIZER_ERROR;
        }
      }
      if(!isdigit(ptr[i])) {
        DEBUG_PRINTF("get_next_token: error due to malformed number\n");
        return TOKENIZER_ERROR;
      }
    }
    DEBUG_PRINTF("get_next_token: error due to too long number\n");
    return TOKENIZER_ERROR;
  } else if(singlechar()) {
    nextptr = ptr + 1;
    return singlechar();
  } else if(*ptr == '"') {
    nextptr = ptr;
    do {
      ++nextptr;
    } while(*nextptr != '"');
    ++nextptr;
    return TOKENIZER_STRING;
  } else {
    for(kt = keywords; kt->keyword != UBASIC_TOKENIZER_ERROR; ++kt) {
      if(ystring_fstrncmp(kt->keyword, ptr, ystring_fstrlen(kt->keyword)) == 0) {
        nextptr = ptr + ystring_fstrlen(kt->keyword);
        return kt->token;
      }
    }
  }

  if(*ptr >= 'a' && *ptr <= 'z') {
    nextptr = ptr + 1;
    return TOKENIZER_VARIABLE;
  }

  return TOKENIZER_ERROR;
}
/*---------------------------------------------------------------------------*/
void
tokenizer_goto(const char *program)
{
  ptr = program;
  current_token = get_next_token();
}
/*---------------------------------------------------------------------------*/
void
tokenizer_init(const char *program)
{
  tokenizer_goto(program);
  current_token = get_next_token();
}
/*---------------------------------------------------------------------------*/
int16_t
tokenizer_token(void)
{
  return current_token;
}
/*---------------------------------------------------------------------------*/
void
tokenizer_next(void)
{

  if(tokenizer_finished()) {
    return;
  }

  DEBUG_PRINTF("tokenizer_next: %p\n", nextptr);
  ptr = nextptr;

  while(*ptr == ' ') {
    ++ptr;
  }
  current_token = get_next_token();

  if(current_token == TOKENIZER_REM) {
      while(!(*nextptr == '\n' || tokenizer_finished())) {
        ++nextptr;
      }
      if(*nextptr == '\n') {
        ++nextptr;
      }
      tokenizer_next();
  }

  DEBUG_PRINTF("tokenizer_next: '%s' %d\n", ptr, current_token);
  return;
}
/*---------------------------------------------------------------------------*/
VARIABLE_TYPE
tokenizer_num(void)
{
//  ytracestr(ptr);
  return ystring_parse_int(ptr, ystring_strlen(ptr));
}
/*---------------------------------------------------------------------------*/
void
tokenizer_string(char *dest, int16_t len)
{
  char *string_end;
  int16_t string_len;

  if(tokenizer_token() != TOKENIZER_STRING) {
    return;
  }
  string_end = strchr(ptr + 1, '"');
  if(string_end == NULL) {
    return;
  }
  string_len = string_end - ptr - 1;
  if(len < string_len) {
    string_len = len;
  }
  memcpy(dest, ptr + 1, string_len);
  dest[string_len] = 0;
}
/*---------------------------------------------------------------------------*/
void
tokenizer_error_print(void)
{
  DEBUG_PRINTF("tokenizer_error_print: '%s'\n", ptr);
}
/*---------------------------------------------------------------------------*/
int16_t
tokenizer_finished(void)
{
  return *ptr == 0 || current_token == TOKENIZER_ENDOFINPUT;
}
/*---------------------------------------------------------------------------*/
int8_t
tokenizer_variable_num(void)
{
  return *ptr - 'a';
}
/*---------------------------------------------------------------------------*/
char const *
tokenizer_pos(void)
{
    return ptr;
}
