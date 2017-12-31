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

#ifndef CONFIG_INCLUDED
#define CONFIG_INCLUDED

#ifdef _WIN32
#include "arch/win32/config_win32.h"
#else
#include "arch/arduino/config_arduino.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define SIM900_POWER_PIN (9)

typedef enum { Y_PIN_MODE_INPUT, Y_PIN_MODE_OUTPUT } pin_mode_t;

void yhal_set_sim900_emu(void *emu);
void yhal_set_pin_state(uint8_t pin, pin_state_t state);
void yhal_set_pin_mode(uint8_t pin, pin_mode_t mode);

uint16_t yhal_read_analog(uint8_t pin);
uint8_t yhal_read_digital(uint8_t pin);

/* returns current vcc voltage in millivolts (max 5000) */
uint16_t yhal_read_vcc();

/* puts CPU into a short (<1s) sleep */
void yhal_enter_ms_sleep();
uint32_t yhal_millis();

typedef enum { CONFIG_PROGRAM_OFFSET } config_elem_t;

int16_t yconfig_read(config_elem_t config_elem);
void yconfig_initialize_if_needed();

#ifdef __cplusplus
}
#endif

#endif
