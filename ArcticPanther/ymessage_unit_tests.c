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

#ifdef RUN_YMESSAGE_UNIT_TESTS
#include "ymessage.h"
#include <config.h>
#include <yassert.h>
#include <yrand.h>
#include <ytimer.h>

#ifndef NDEBUG

static bool is_message_ready = false;
#define BUFF_LEN (49)

void message_ready_cb() { is_message_ready = true; }

#endif

void ymessage_unit_tests(void) {
#ifndef NDEBUG
  uint8_t buff[BUFF_LEN];
  uint64_t payload = 0x1234567890abcdef;

  payload = 10 + ((uint64_t)10 << 32); // TODO - read from sensor

  is_message_ready = false;
  ytimer_set_uptime_in_ms((uint64_t)0x123456789abcdef);
  yrand_seed(1, 1);
  yassert(Y_OK ==
          ymessage_construct(buff, BUFF_LEN, payload, message_ready_cb));
  yassert(is_message_ready == true);

  is_message_ready = false;
  payload = 10 + ((uint64_t)10 << 32); // TODO - read from sensor
  yassert(Y_OK ==
          ymessage_construct(buff, BUFF_LEN, payload, message_ready_cb));
  yassert(is_message_ready == true);

#endif
}
#endif
