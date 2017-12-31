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

#include "ytimer.h"
#include <stdint.h>
#include <yassert.h>

typedef struct {
  ytimer_cb callback;
  uint64_t when_to_call;
} callback_entry_t;

static struct {
  uint64_t current_ms;
  uint32_t prev_ms;
  uint32_t active_cb_map;
  uint64_t skew;
  callback_entry_t callbacks[32];
} it;

static bool initialized = false;

static uint32_t get_power_of_2(uint8_t power) {
  yassert(power < 32);
  return (uint32_t)1 << power;
}

/**
\brief Sets up a timeout which is going to be fired in a specific time
*/
timer_id_t ytimer_setup_onetime_callback(ytimer_cb callback,
                                         uint32_t delay_in_ms) {
  uint8_t i;

  yassert(sizeof(it.active_cb_map) ==
          4); /* this logic relies on the callback size value */
  yassert(it.active_cb_map != UINT32_MAX);

  for (i = 0; i < 32; i++) {
    uint32_t mask = get_power_of_2(i);
    if ((it.active_cb_map & mask) == 0) {
      /* free spot found */
      it.active_cb_map |= mask;

      it.callbacks[i].callback = callback;
      it.callbacks[i].when_to_call = ytimer_get_uptime_in_ms() + delay_in_ms;
      break;
    }
  }

  return (timer_id_t)i;
}

void ytimer_reset_onetime_callback(timer_id_t idx) {
  uint32_t mask = get_power_of_2(idx);
  it.active_cb_map &= ~mask;
}

/**
\brief gets the uptime in milliseconds
*/
uint64_t ytimer_get_uptime_in_ms() { return it.current_ms + it.skew; }

static void ytimer_iterate_and_call_callbacks(uint64_t prev_check) {
  uint8_t i;

  uint64_t time = ytimer_get_uptime_in_ms();

  yassert(sizeof(it.active_cb_map) ==
          4); /* this logic relies on the callback size value */

  for (i = 0; i < 32; i++) {
    uint32_t mask = get_power_of_2(i);
    if (it.active_cb_map & mask) {
      /* active callback found spot found */
      if (it.callbacks[i].when_to_call <= time) {
        it.active_cb_map &= ~mask;
        it.callbacks[i].callback();
      }
    }
  }
}

/**
\brief Provides time measurement and executes the main timer loop

\param current_ms current time in millis; if less than previously provided it
means the uint32_t rolled over
*/
void ytimer_tick(uint32_t current_ms) {
  uint64_t prev_check;

  if (!initialized) {
    initialized = true;
    it.active_cb_map = 0;
    it.prev_ms = 0;
    it.current_ms = 0;
  }

  prev_check = it.current_ms;

  if (current_ms < it.prev_ms) {
    /*
    time rollback, it will happen once per UINT32_MAX / (1000*60s*60min*24) days
    --> once per 49 days

    the below equation adds carryover to the current_ms, which for UINT32_MAX
    (0xffffffff) is 0x100000000 the rest of the time will be filled by a new
    time, just after the if
    */
    it.current_ms += (uint64_t)UINT32_MAX + 1;
    it.current_ms &= ~((uint64_t)UINT32_MAX);
    it.prev_ms = 0;
  }

  it.current_ms += current_ms - it.prev_ms;
  it.prev_ms = current_ms;

  ytimer_iterate_and_call_callbacks(prev_check);
}

/**
\brief Resets the internal timer

\param current_uptime_ms new time to be set
*/
void ytimer_set_uptime_in_ms(uint64_t current_uptime_ms) {
  it.current_ms = current_uptime_ms;
  it.prev_ms = (uint32_t)(current_uptime_ms);
  it.skew = 0;
}

void ytimer_force_init() { initialized = false; }

uint32_t ytimer_millis() { return yhal_millis() + it.skew; }

void ytimer_millis_add_skew(int32_t skew) { it.skew += skew; }
