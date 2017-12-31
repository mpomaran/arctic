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

#include "tests.h"

#ifdef RUN_YSTRING_UNIT_TESTS

#include "yassert.h"
#include "ystring.h"

#ifndef NDEBUG

void test_ystring_get_char_idx() {
  yassert(ystring_get_char_idx("", 2, 's') < 0);
  yassert(ystring_get_char_idx("", 0, 's') < 0);
  yassert(ystring_get_char_idx("123", 0, 's') < 0);
  yassert(ystring_get_char_idx("123", 2, 's') < 0);
  yassert(ystring_get_char_idx("123", 3, '1') == 0);
  yassert(ystring_get_char_idx("123", 3, '2') == 1);
  yassert(ystring_get_char_idx("123", 3, '3') == 2);
  yassert(ystring_get_char_idx("123", 13, '3') == 2);
}

const char program_let[] = "10 let a = 42\n\
		20 end\n";

void test_ystring_parse_int() {
  yassert(ystring_parse_int(program_let, ystring_strlen(program_let)) == 10);
  yassert(ystring_parse_int("123", 3) == 123);
  yassert(ystring_parse_int("123", 2) == 12);
  yassert(ystring_parse_int(",123", 2) == 1);
  yassert(ystring_parse_int(",123", 12) == 123);
  yassert(ystring_parse_int(",123,", 12) == 123);
  yassert(ystring_parse_int("1", 12) == 1);
  yassert(ystring_parse_int(",1", 12) == 1);
  yassert(ystring_parse_int("1", 1) == 1);
  yassert(ystring_parse_int(",1", 1) == 0);
}

void test_ystring_equals() {
  yassert(ystring_equals(NULL, NULL, 0) == true);
  yassert(ystring_equals("a", "a", 1) == true);
  yassert(ystring_equals("a", "a", 10) == true);
  yassert(ystring_equals("ab", "a", 1) == true);
  yassert(ystring_equals("a", "ab", 1) == true);
  yassert(ystring_equals("aa", "aa", 2) == true);
  yassert(ystring_equals("aab", "aaa", 2) == true);
  yassert(ystring_equals("aa", "aab", 2) == true);
  yassert(ystring_equals("a", "b", 1) == false);
}

void test_ystring_copy_fstring() {
  uint8_t len;

  len = ystring_cpy_fstring_to_buff(TEST_STRING_A);
  yassert(len == 1);
  yassert(ystring_strlen(ystring_buff) == len);
  yassert(ystring_equals("|", ystring_buff, len));

  len = ystring_cpy_fstring_to_buff(TEST_STRING_BB);
  yassert(len == 2);
  yassert(ystring_strlen(ystring_buff) == len);
  yassert(ystring_equals("!!", ystring_buff, len));
}

void test_ystring_append() {
  ystring_cpy_fstring_to_buff(TEST_STRING_A);
  ystring_append(ystring_buff, YSTRING_BUFF_LEN, ystring_buff);
  yassert(ystring_strlen(ystring_buff) == 2);
}

void test_ystring_fstrncmp() {
  ystring_cpy_fstring_to_buff(TEST_STRING_BB);
  yassert(ystring_fstrncmp(TEST_STRING_BB, ystring_buff,
                           ystring_fstrlen(TEST_STRING_BB)) == 0);
  ystring_buff[1] = 'v';
  yassert(ystring_fstrncmp(TEST_STRING_BB, ystring_buff,
                           ystring_fstrlen(TEST_STRING_BB)) != 0);
  yassert(ystring_fstrncmp(TEST_STRING_A, ystring_buff,
                           ystring_fstrlen(TEST_STRING_A)) != 0);
  yassert(ystring_fstrncmp(TEST_STRING_A, NULL,
                           ystring_fstrlen(TEST_STRING_A)) != 0);
}

void test_ystring_fstrlen() {
  yassert(ystring_fstrlen(TEST_STRING_BB) == 2);
  yassert(ystring_fstrlen(TEST_STRING_A) == 1);
}

#endif
void ystring_unit_tests(void) {
#ifndef NDEBUG
  test_ystring_get_char_idx();
  test_ystring_parse_int();
  test_ystring_equals();
  test_ystring_copy_fstring();
  test_ystring_append();
  test_ystring_fstrncmp();
  test_ystring_fstrlen();
#endif
}

#endif
