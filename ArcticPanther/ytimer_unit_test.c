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

#ifdef RUN_YTIMER_UNIT_TESTS

#include "yassert.h"
#include "ytimer.h"

#include <stdint.h>

#define CALLBACK_TIMEOUT_MS 10
static int callbacks_count = 0;

/* callback tests assume the internal implementation did not change - ids are
 * assigned in order of adding callbacks*/
void timeout() { callbacks_count++; }

static void ytimer_setup_onetime_callback_test() {
  int i = 0;

  callbacks_count = 0;

  ytimer_set_uptime_in_ms(0);

  for (i = 0; i < 32; i++) {
    timer_id_t timer_id = ytimer_setup_onetime_callback(timeout, i);

    yassert(timer_id == i);
  }

  for (i = 0; i < 32; i++) {
    ytimer_tick(i);
  }

  for (i = 0; i < 32; i++) {
    yassert(ytimer_setup_onetime_callback(timeout, i) == i);
  }

  for (i = 0; i < 32; i++) {
    ytimer_tick(i);
  }

  yassert(callbacks_count == 64);
}

static void ytimer_set_get_uptime_in_ms_test() {
  ytimer_set_uptime_in_ms(0);
  yassert(ytimer_get_uptime_in_ms() == 0);

  ytimer_tick(UINT32_MAX - 1);
  yassert(ytimer_get_uptime_in_ms() == UINT32_MAX - 1);

  ytimer_tick(1);
  yassert(ytimer_get_uptime_in_ms() == (uint64_t)UINT32_MAX + 2);

  ytimer_set_uptime_in_ms(0);
  ytimer_tick(1);
  ytimer_tick(42);
  yassert(ytimer_get_uptime_in_ms() == 42);
}

static void ytimer_setup_1000ms_callback_test() {
  ytimer_set_uptime_in_ms(0);
  int i = 0;
  callbacks_count = 0;

  ytimer_setup_onetime_callback(timeout, 1000);

  for (i = 0; i <= 1000; i++) {
    ytimer_tick(i);
  }

  yassert(callbacks_count == 1);
}

static void ytimer_reset_timeout_test() {
  uint8_t to_cancel;
  ytimer_set_uptime_in_ms(0);
  int i = 0;
  callbacks_count = 0;

  ytimer_setup_onetime_callback(timeout, 1000);
  to_cancel = ytimer_setup_onetime_callback(timeout, 1000);
  ytimer_setup_onetime_callback(timeout, 1000);
  ytimer_reset_onetime_callback(to_cancel);

  for (i = 0; i <= 1000; i++) {
    ytimer_tick(i);
  }

  yassert(callbacks_count == 2);
}

void ytimer_execute_unit_tests() {
  ytimer_set_get_uptime_in_ms_test();
  ytimer_setup_onetime_callback_test();
  ytimer_setup_1000ms_callback_test();
  ytimer_reset_timeout_test();
}

#endif
