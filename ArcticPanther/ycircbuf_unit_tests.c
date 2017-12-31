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

#ifdef RUN_YCIRCBUF_UNIT_TESTS

#include "yassert.h"
#include "ycircbuf.h"

#define BUFF_CAPACITY (8)

static ycircbuf_t test_alloc() {
  ycircbuf_t buff = ycircbuf_alloc(BUFF_CAPACITY);
  yassert(buff != 0);

  return buff;
}

static test_capacity(ycircbuf_t buf) {
  yassert(ycircbuf_get_capacity(buf) == BUFF_CAPACITY);
}

static void test_write(ycircbuf_t buf) {
  uint8_t i = 0;

  for (i = 0; i < BUFF_CAPACITY / 2; i++) {
    yassert(ycircbuf_append(buf, &i, 1) == 1);
  }
}

static void test_read(ycircbuf_t buf) {
  size_t i = ycircbuf_size(buf);
  uint8_t j;

  j = 0;
  for (; i > 0; i--) {
    uint8_t b;
    yassert(ycircbuf_pop(buf, &b, 1) == 1);
    yassert(b == j);
    j++;
  }

  yassert(ycircbuf_size(buf) == 0);
}

static void test_free(ycircbuf_t buf) { ycircbuf_free(buf); }

static void test_overfill(ycircbuf_t buf) {
  uint8_t i, j;

  for (i = 0; i < BUFF_CAPACITY * 2; i++) {
    if (ycircbuf_append(buf, &i, 1) == 0) {
      break;
    }
  }

  yassert(i == BUFF_CAPACITY);
  for (j = 0; j < i; j++) {
    uint8_t b;
    yassert(ycircbuf_size(buf) == BUFF_CAPACITY - j);
    yassert(ycircbuf_pop(buf, &b, 1) == 1);
    yassert(b == j);
  }
}

void ycircbuf_execute_unit_test() {
  ycircbuf_t buf = test_alloc();
  int i;

  test_capacity(buf);
  test_overfill(buf);

  for (i = 0; i < 100; i++) {
    test_write(buf);
    test_read(buf);
  }

  test_free(buf);
}

#endif
