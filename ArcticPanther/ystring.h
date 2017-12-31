/**

MIT License

Copyright (c) 2017 mpomaran (mpomaranski at gmail com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#ifndef YSTRING_H_HAS_BEEN_INCLUDED
#define YSTRING_H_HAS_BEEN_INCLUDED

#include <config.h>

#ifdef __cplusplus
extern "C" {
#endif

#define YSTRING_BUFF_LEN (128)

extern char ystring_buff[];

typedef enum {
  TEST_STRING_A = 0,
  TEST_STRING_BB,
  ATT_ATE0,
  ATT_AT,
  ATT_OK,
  ATT_CEER_0,
  ATT_CSQ,
  ATT_CGATT_1,
  ATT_CGDCONT_1,
  ATT_SAPBR_CONTYPE,
  ATT_SAPBR_APN,
  ATT_SAPBR_USER,
  ATT_SAPBR_PASSWD,
  ATT_SAPBR_SET,
  ATT_CGATT_0,
  ATT_CREG,
  ATT_CREG_0,
  ATT_COPS,
  ATT_HTTPINIT,
  ATT_HTTPARA_URL_PREFIX,
  ATT_HTTP_ACTION_0,
  ATT_HTTP_READ,
  ATT_HTTP_TERM,
  ATT_CMEE_2,
  ATT_HTTPPARA_SET_CONTEXT_ID,

  UBASIC_TOKENIZER_LET,
  UBASIC_TOKENIZER_PRINT,
  UBASIC_TOKENIZER_IF,
  UBASIC_TOKENIZER_THEN,
  UBASIC_TOKENIZER_ELSE,
  UBASIC_TOKENIZER_FOR,
  UBASIC_TOKENIZER_TO,
  UBASIC_TOKENIZER_NEXT,
  UBASIC_TOKENIZER_GOTO,
  UBASIC_TOKENIZER_GOSUB,
  UBASIC_TOKENIZER_RETURN,
  UBASIC_TOKENIZER_CALL,
  UBASIC_TOKENIZER_REM,
  UBASIC_TOKENIZER_PEEK,
  UBASIC_TOKENIZER_POKE,
  UBASIC_TOKENIZER_END,
  UBASIC_TOKENIZER_ERROR,

  FSTRING_COUNT
} fstring_id;

int16_t ystring_get_char_idx(const uint8_t *str, uint16_t len, uint8_t c);
uint16_t ystring_parse_int(const uint8_t *str, uint16_t len);
bool ystring_equals(const uint8_t *a, const uint8_t *b, uint8_t len);

/**
\brief Returns the given ASCII character from the command

\param command the command id
\param idx index of the character in the command string

\return character at the position idx of the command string or -1 if idx is out
of bounds
*/
int ystring_get_char_from_fstring(fstring_id command, uint8_t idx);

/**
\brief getter designed for parser
*/
int ystring_fstring_getter(int cookie, int command, int idx);

/**
\brief Copies the fstring to static buffer; it is a static variable allocated
inside the ysting implementation and should be read only for others

\param id of the fstring
\return length of the string
*/
uint8_t ystring_cpy_fstring_to_buff(fstring_id fstring);

/**
\brief compares (without case sensivity) strings with stored string value
and returns 0 if matches, otherwise like in the strncmp
*/
int8_t ystring_fstrncmp(fstring_id fstring, const char *str, uint8_t len);

void ystring_memzero(char *buff, uint8_t len);

uint8_t ystring_strlen(const char *buff);

uint8_t ystring_fstrlen(fstring_id fstring);

void ystring_append(char *dst, uint8_t dst_buff_len, const char *src);

#ifdef __cplusplus
}
#endif

#endif
