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

#ifdef RUN_YSERIAL_UNIT_TESTS
#include <assert.h>
#include <memory.h>
#include <string.h>

#include "yassert.h"
#include "yserial.h"

static const char *test_string = "T";

static void test_for_2_bytes(yserial_t serial, void *cookie,
                             const uint8_t *buff, size_t buff_len) {
  yassert(cookie == test_string);
  yassert(buff_len == strlen(test_string) + 1);
  yassert(memcmp(buff, test_string, buff_len) == 0);
}

static void test_for_1_byte(yserial_t serial, void *cookie, const uint8_t *buff,
                            size_t buff_len) {
  yassert(buff_len == strlen(test_string));
  yassert(memcmp(buff, test_string, buff_len) == 0);
}

void yserial_execute_unit_tests() {
  char *buff = malloc(strlen(test_string) + 1);
  int i = 0;
  yserial_t serial = yserial_alloc(19200, 7, 8);

  yserial_test_set_fake_mode(serial, test_for_2_bytes, test_string);
  while (yserial_available(serial)) {
    yserial_read_byte(serial);
  }

  yserial_write_data(serial, (const uint8_t *)test_string,
                     strlen(test_string) + 1);

  yserial_test_set_fake_mode(serial, test_for_1_byte, test_string);
  yserial_write_string(serial, test_string);

  yserial_test_simulate_received(serial, (const uint8_t *)test_string,
                                 strlen(test_string));

  for (i = 0; i < (int)strlen(test_string); i++) {
    assert(yserial_available(serial));
    assert(test_string[i] == yserial_read_byte(serial));
  }
  assert(yserial_available(serial) == false);

  yserial_test_simulate_received(serial, (const uint8_t *)test_string,
                                 strlen(test_string) + 1);
  assert(yserial_read_bytes(serial, buff, strlen(test_string) + 1) ==
         strlen(test_string) + 1);
  assert(memcmp(buff, test_string, strlen(test_string) + 1) == 0);

  yserial_test_set_fake_mode(serial, NULL, NULL);

  free(buff);
}

#endif
