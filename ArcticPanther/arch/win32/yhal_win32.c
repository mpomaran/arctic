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

#include "config_win32.h"
#include "ysim900_emulator.h"
#include <tests.h>

#ifdef _WIN32
#include <windows.h>
#elif _POSIX_C_SOURCE >= 199309L
#include <time.h> // for nanosleep
#else
#include <unistd.h> // for usleep
#endif

static ysim900_emu_t sim900;

void yhal_set_sim900_emu(void *emu) { sim900 = (ysim900_emu_t)emu; }

void yhal_set_pin_mode(uint8_t pid, pin_mode_t mode) {
  // NOOP for _WIN32
}

void yhal_set_pin_state(uint8_t pin, pin_state_t state) {
#ifndef RUN_YGSMMODEM_UNIT_TESTS
// NOP
#else
  if (pin == SIM900_POWER_PIN && sim900 != NULL) {
    ysim900_emu_set_power_control_pin(sim900, state);
  }
#endif
}

void ytimer_delay(uint32_t milliseconds) // cross-platform sleep function
{
#ifdef _WIN32
  Sleep(milliseconds);
#elif _POSIX_C_SOURCE >= 199309L
  struct timespec ts;
  ts.tv_sec = milliseconds / 1000;
  ts.tv_nsec = (milliseconds % 1000) * 1000000;
  nanosleep(&ts, NULL);
#else
  usleep(milliseconds * 1000);
#endif
}

/* puts CPU into a short (<1s) sleep */
void yhal_enter_ms_sleep() { ytimer_delay(1); }

uint8_t yhal_read_digital(uint8_t pin) { return pin & 0x01; }

uint16_t yhal_read_analog(uint8_t pin) { return pin; }

uint32_t yhal_millis() { return 0; }

void yconfig_initialize_if_needed() {
  // TODO
}

uint16_t yhal_read_vcc() { return 5000; }
