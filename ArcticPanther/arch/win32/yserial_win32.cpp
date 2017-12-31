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

#include <list>
#include <stdint.h>

#include "config.h"
#include <yassert.h>
#include <yserial.h>
#include <ystring.h>

using namespace std;

#ifndef _WIN32
#error This file is for Windows only!!!
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct yserial {
  yserial_test_data_receiver_callback_fn callback;
  list<uint8_t> input;
  void *cookie;
};

yserial_t yserial_alloc(uint32_t baud_speed, uint8_t rx_pin, uint8_t tx_pin) {
  yserial_t result =
      (struct yserial *)new yserial(); //(sizeof(struct yserial));
  return result;
}

yerrno_t yserial_free(yserial_t serial) {
  delete serial;
  return Y_OK;
}

bool yserial_available(yserial_t serial) {
  return serial != NULL && !serial->input.empty();
}

uint8_t yserial_read_byte(yserial_t serial) {
  list<uint8_t> &input = serial->input;
  assert(!input.empty());
  uint8_t result = *(input.begin());
  input.pop_front();
  return result;
}

size_t yserial_read_bytes(yserial_t serial, uint8_t *buff, size_t buff_len) {
  unsigned i;
  for (i = 0; i < buff_len; i++) {
    buff[i] = yserial_read_byte(serial);
  }
  return i;
}

size_t yserial_write_data(yserial_t serial, const uint8_t *buff,
                          size_t buff_len) {
  serial->callback(serial, serial->cookie, buff, buff_len);
  return buff_len;
}

size_t yserial_write_string(yserial_t serial, const char *str) {
  return yserial_write_data(serial, (const uint8_t *)str, strlen(str));
}

void yserial_write_fstring(yserial_t serial, fstring_id command) {
  int16_t c;
  uint8_t i = 0;

  yassert(command < FSTRING_COUNT);

  while (c = ystring_get_char_from_fstring(command, i++) != -1) {
    uint8_t b = (uint8_t)c;
    yserial_write_data(serial, &b, 1);
  }
}

/**
\brief TEST API - when called then adds to the receiving queue specifie data,
which can be read by client
* as received from external device
*/
size_t yserial_test_simulate_received(yserial_t serial, const uint8_t *buff,
                                      size_t buff_len) {
  unsigned i;
  for (i = 0; i < buff_len; i++) {
    serial->input.push_back(buff[i]);
    if (buff[i] == 0) {
      break;
    }
  }

  return i;
}

/**
\bref TEST API - sets callback which is to be called each client user uses
"write"
*/
void yserial_test_set_fake_mode(yserial_t serial,
                                yserial_test_data_receiver_callback_fn cb,
                                void *cookie) {
  serial->callback = cb;
  serial->cookie = cookie;
}

#ifdef __cplusplus
}
#endif
