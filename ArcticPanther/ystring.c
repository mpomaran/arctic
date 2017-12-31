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

#include "ystring.h"
#include "yassert.h"

char ystring_buff[YSTRING_BUFF_LEN];

/* todo remove end of lines */

const char progmem_str_1[] PROGMEM = "|";
const char progmem_str_2[] PROGMEM = "!!";
const char progmem_str_3[] PROGMEM = "ATE0";
const char progmem_str_4[] PROGMEM = "AT\r\n";
const char progmem_str_5[] PROGMEM = "OK\r\n";
const char progmem_str_6[] PROGMEM = "AT+CEER=0\r\n";
const char progmem_str_7[] PROGMEM = "AT+CSQ\r\n";
const char progmem_str_8[] PROGMEM = "AT+CGATT=1\r\n";
const char progmem_str_9[] PROGMEM =
    "AT+CGDCONT=1,\"IP\",\"internet GPRS\", \"0.0.0.0\"\r\n";
const char progmem_str_10[] PROGMEM = "AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n";
const char progmem_str_11[] PROGMEM = "AT+SAPBR=3,1,\"APN\",\"Internet\"\r\n";
const char progmem_str_12[] PROGMEM = "AT+SAPBR=3,1,\"USER\",\"Internet\"\r\n";
const char progmem_str_13[] PROGMEM = "AT+SAPBR=3,1,\"PWD\",\"Internet\"\r\n";
const char progmem_str_14[] PROGMEM = "AT+SAPBR=1,1";
const char progmem_str_15[] PROGMEM = "AT+CGATT=0\r\n";
const char progmem_str_16[] PROGMEM = "AT+CREG?\r\n";
const char progmem_str_17[] PROGMEM = "AT+CREG=0\r\n";
const char progmem_str_18[] PROGMEM = "AT+COPS?\r\n";
const char progmem_str_19[] PROGMEM = "AT+HTTPINIT\r\n";
const char progmem_str_20[] PROGMEM = "AT+HTTPPARA=\"URL\",\"";
const char progmem_str_21[] PROGMEM = "AT+HTTPACTION=0";
const char progmem_str_22[] PROGMEM = "AT+HTTPREAD";
const char progmem_str_23[] PROGMEM = "AT+HTTPTERM";
const char progmem_str_24[] PROGMEM = "AT+CMEE=2";
const char progmem_str_25[] PROGMEM = "AT+HTTPPARA = \"CID\", 1";

const char progmem_str_26[] PROGMEM = "let";
const char progmem_str_27[] PROGMEM = "print";
const char progmem_str_28[] PROGMEM = "if";
const char progmem_str_29[] PROGMEM = "then";
const char progmem_str_30[] PROGMEM = "else";
const char progmem_str_31[] PROGMEM = "for";
const char progmem_str_32[] PROGMEM = "to";
const char progmem_str_33[] PROGMEM = "next";
const char progmem_str_34[] PROGMEM = "goto";
const char progmem_str_35[] PROGMEM = "gosub";
const char progmem_str_36[] PROGMEM = "return";
const char progmem_str_37[] PROGMEM = "call";
const char progmem_str_38[] PROGMEM = "rem";
const char progmem_str_39[] PROGMEM = "peek";
const char progmem_str_40[] PROGMEM = "poke";
const char progmem_str_41[] PROGMEM = "end";
const char progmem_str_42[] PROGMEM = "";

const char *const progmem_table[] PROGMEM = {
    progmem_str_1,  progmem_str_2,  progmem_str_3,  progmem_str_4,
    progmem_str_5,  progmem_str_6,  progmem_str_7,  progmem_str_8,
    progmem_str_9,  progmem_str_10, progmem_str_11, progmem_str_12,
    progmem_str_13, progmem_str_14, progmem_str_15, progmem_str_16,
    progmem_str_17, progmem_str_18, progmem_str_19, progmem_str_20,
    progmem_str_21, progmem_str_22, progmem_str_23, progmem_str_24,
    progmem_str_25, progmem_str_26, progmem_str_27, progmem_str_28,
    progmem_str_29, progmem_str_30, progmem_str_31, progmem_str_32,
    progmem_str_33, progmem_str_34, progmem_str_35, progmem_str_36,
    progmem_str_37, progmem_str_38, progmem_str_39, progmem_str_40,
    progmem_str_41, progmem_str_42};

int16_t ystring_get_char_idx(const uint8_t *str, uint16_t len, uint8_t c) {
  uint16_t result = 0;

  while ((result < len) && (str[result] != c) && (str[result] != 0)) {
    result++;
  }

  if (len == result || str[result] == 0) {
    return -1;
  } else {
    return result;
  }
}

uint16_t ystring_parse_int(const uint8_t *str, uint16_t len) {
  uint16_t result = 0;
  uint16_t idx = 0;

  while ((str[idx] < '0' || str[idx] > '9') && idx < len) {
    idx++;
  }

  while ((str[idx] >= '0' && str[idx] <= '9') && idx < len) {
    result = result * 10 + (str[idx] - '0');
    idx++;
  }

  return result;
}

/**
 * Compares bytes
 * \param a string a to compare
 * \param b string b to compare
 * \param len max length of strings
 *
 * returns true is equal or false
 */
bool ystring_equals(const uint8_t *a, const uint8_t *b, uint8_t len) {
  uint8_t i = 0;
  for (i = 0; i < len; i++) {
    if (a[i] != b[i])
      return false;

    if (a[i] == 0)
      break;
  }

  return true;
}

static uint8_t ystring_fstrlen_internal(const char *str) {
  uint8_t result = 0;

  for (; read_uint8(str + result) != 0 && result < 128; result++) {
    ;
  }
  return result;
}

int ystring_get_char_from_fstring(fstring_id command, uint8_t idx) {
  if (command >= FSTRING_COUNT) {
    return -1;
  }

  char *str = (char *)read_ptr(&(progmem_table[command]));

  if (idx < (int)ystring_fstrlen_internal(str)) {
    return read_uint8(str + idx);
  } else {
    return -1;
  }
}

int ystring_fstring_getter(int cookie, int command, int idx) {
  return ystring_get_char_from_fstring(command, idx);
}

char ystring_to_lower(const char c) {
  if (c >= 'a' && c <= 'z') {
    return c;
  } else if (c >= 'A' && c <= 'Z') {
    return c + ('a' - 'A');
  } else {
    return c;
  }
}

int8_t ystring_fstrncmp(fstring_id fstring, const char *str, uint8_t len) {
  uint8_t idx = 0;
  int fstring_c;
  char str_c;

  if (str == NULL)
    return 1;
  if (fstring == FSTRING_COUNT)
    return -1;

  do {
    fstring_c = ystring_get_char_from_fstring(fstring, idx);
    str_c = *(str + idx);

    if (ystring_to_lower(fstring_c) != ystring_to_lower(str_c)) {
      return ystring_to_lower(fstring_c) - ystring_to_lower(str_c);
    }
    idx++;
  } while (fstring_c != -1 && str_c != 0 && idx < len);
  return 0;
}

void ystring_memzero(char *buff, uint8_t len) {
  if (len > 0)
    buff[0] = 0;

  while (len != 0) {
    buff[--len] = 0;
  }
}

uint8_t ystring_strlen(const char *buff) {
  uint8_t result = 0;
  while ((result < 0xff) && (buff[result] != 0)) {
    result++;
  }
  return result;
}

uint8_t ystring_fstrlen(fstring_id fstring) {
  uint8_t result = 0;
  while ((result < 0xff) &&
         (ystring_get_char_from_fstring(fstring, result) != -1)) {
    result++;
  }
  return result;
}

uint8_t ystring_cpy_fstring_to_buff(fstring_id fstring) {
  uint8_t result = 0;
  int16_t c;

  ystring_memzero(ystring_buff, YSTRING_BUFF_LEN);

  do {
    c = ystring_get_char_from_fstring(fstring, result);
    if (c >= 0) {
      ystring_buff[result] = (uint8_t)c;
      result++;
    }
  } while (result < YSTRING_BUFF_LEN && c > 0);

  return result;
}

void ystring_append(char *dst, uint8_t dst_buff_len, const char *src) {
  uint8_t dst_idx, src_idx;
  uint8_t src_len;

  yassert(dst != NULL && dst_buff_len != 0 && src != 0);

  src_len = ystring_strlen(src);

  for (src_idx = 0, dst_idx = ystring_strlen(dst);
       (dst_idx < dst_buff_len - 1) && (src_idx < src_len);
       dst_idx++, src_idx++) {
    dst[dst_idx] = src[src_idx];
  }
  dst[dst_idx] = 0;
}
