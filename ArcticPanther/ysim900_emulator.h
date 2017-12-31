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

#include "yserial.h"
#include "ytypes.h"

#ifndef YSIM900_EMULATOR_H_HAS_BEEN_INCLUDED
#define YSIM900_EMULATOR_H_HAS_BEEN_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

struct ysim900_emu;
typedef struct ysim900_emu *ysim900_emu_t;

ysim900_emu_t ysim900_emu_alloc(yserial_t serial);

// sets if initial state is on
yerrno_t ysim900_emu_set_is_on(ysim900_emu_t emu, bool is_on);

// sets if initial state is off
yerrno_t ysim900_emu_set_power_control_pin(ysim900_emu_t emu,
                                           pin_state_t state);

void ysim900_emu_tick(ysim900_emu_t emu);
yerrno_t ysim900_explode_on_error(ysim900_emu_t emu, bool should_explode);

yerrno_t ysim900_emu_free(ysim900_emu_t emu);
bool ysim900_exploded(ysim900_emu_t emu);

#ifdef __cplusplus
}
#endif

#endif
