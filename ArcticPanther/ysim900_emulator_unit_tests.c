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

#ifdef RUN_YSIM900_EMU_UNIT_TESTS

#include "ysim900_emulator.h"

#include "yassert.h"
#include "yparser.h"
#include "yserial.h"
#include "ystring.h"
#include "ytimer.h"
#include "ytypes.h"

/*
Story 1: Power control
*/

/*
Scenario 1: It should be allowed to power on powered off modem

Given:
        Modem is powered off
        Power control pin state is low
        There is serial connection
When:
        High state on the power control pin is high for 2 seconds followed by
permanent low state Then: Modem is powered on after 5 seconds Some random
characteds are emmited for couple of seconds after high state is set to down
followed by "OK" string
*/
static void should_allow_to_power_on_modem() {
  // given
  yserial_t serial = yserial_alloc(19200, 7, 8);

  ysim900_emu_t emu = ysim900_emu_alloc(serial);
  ysim900_emu_set_is_on(emu, false);
  yhal_set_sim900_emu(emu);
  yhal_set_pin_state(SIM900_POWER_PIN, Y_LOW);

  ytimer_set_uptime_in_ms(0);
  ytimer_tick(0);
  ysim900_emu_tick(emu);

  ytimer_tick(1000);
  ysim900_emu_tick(emu);
  yassert(yserial_available(serial) == false);

  // when
  yhal_set_pin_state(SIM900_POWER_PIN, Y_HIGH);
  ytimer_tick(3500);
  ysim900_emu_tick(emu);
  yhal_set_pin_state(SIM900_POWER_PIN, Y_LOW);
  ytimer_tick(6000);
  ysim900_emu_tick(emu);

  // then
  ytimer_tick(8000);
  yassert(yserial_available(serial) == true);
  while (yserial_available(serial)) {
    yserial_read_byte(serial);
  }
  ytimer_tick(80000);
  yassert(yserial_available(serial) == false);

  yhal_set_sim900_emu(NULL);
  ysim900_emu_free(emu);
  yserial_free(serial);
}

/*
Scenario 1: It should not be allowed to power on powered off modem if pulse in
pin is too short

Given:
Modem is powered off
Power control pin state s low
There is serial connection
When:
High state on the power control pin is high for 1 second followed by permanent
low state Then: Modem is powered off No output on serial
*/
static void should_not_allow_to_power_on_modem_if_pulse_is_too_short() {
  // given
  yserial_t serial = yserial_alloc(19200, 7, 8);

  ysim900_emu_t emu = ysim900_emu_alloc(serial);
  ysim900_emu_set_is_on(emu, false);
  yhal_set_pin_state(SIM900_POWER_PIN, Y_LOW);
  yhal_set_sim900_emu(emu);

  ytimer_set_uptime_in_ms(0);
  ytimer_tick(0);
  ysim900_emu_tick(emu);

  ytimer_tick(1000);
  ysim900_emu_tick(emu);
  yassert(yserial_available(serial) == false);

  // when
  yhal_set_pin_state(SIM900_POWER_PIN, Y_HIGH);
  ytimer_tick(2500);
  ysim900_emu_tick(emu);
  yhal_set_pin_state(SIM900_POWER_PIN, Y_LOW);
  ytimer_tick(6000);
  ysim900_emu_tick(emu);

  // then
  ytimer_tick(8000);
  yassert(yserial_available(serial) == false);

  yhal_set_sim900_emu(NULL);
  ysim900_emu_free(emu);
  yserial_free(serial);
}

/*
Scenario: Should not allow sending serial commands when power if off

Given:
Modem is powered off

When:
Anything is sent over serial

Then:
Exception is raised
*/
static void should_allow_to_process_when_modem_is_powered_off() {
  yserial_t serial = yserial_alloc(19200, 7, 8);

  ysim900_emu_t emu = ysim900_emu_alloc(serial);
  ysim900_emu_set_is_on(emu, false);
  yhal_set_sim900_emu(emu);
  yhal_set_pin_state(SIM900_POWER_PIN, Y_LOW);
  ytimer_set_uptime_in_ms(0);

  ytimer_tick(0);
  ysim900_emu_tick(emu);
  ysim900_explode_on_error(emu, false);

  ytimer_tick(1000);
  ysim900_emu_tick(emu);
  yassert(ysim900_exploded(emu) == false);

  // when
  yserial_write_data(serial, (const uint8_t *)&serial, sizeof(yserial_t *));
  ytimer_tick(8000);

  // then
  yassert(ysim900_exploded(emu) == false);

  yhal_set_sim900_emu(NULL);
  ysim900_emu_free(emu);
  yserial_free(serial);
}

/*
Scenario: after receiving ATE0 OK should be returned
Given:
        Modem is powered on
When:
        ATE0 is received
Then:
        Modem should return "OK"
*/
void should_allow_ate0_command_after_power_up() {
  // given
  yserial_t serial = yserial_alloc(19200, 7, 8);
  yparser_t parser = yparser_alloc(FSTRING_COUNT, 0, ystring_fstring_getter);

  ysim900_emu_t emu = ysim900_emu_alloc(serial);
  ysim900_emu_set_is_on(emu, true);
  yhal_set_sim900_emu(emu);
  ytimer_set_uptime_in_ms(0);
  ytimer_tick(0);
  yparser_reset(parser);
  int result = YPARSER_RESULT_NO_MATCH_FOUND;

  // when
  yserial_write_string(serial, "ATE0\r\n");
  ytimer_tick(10);
  ysim900_emu_tick(emu);

  // then
  yassert(yserial_available(serial) == true);
  while (yserial_available(serial)) {
    uint8_t b = yserial_read_byte(serial);
    result = yparser_next(parser, b);
  }

  yassert(result == ATT_OK);
  yassert(yserial_available(serial) == false);

  yhal_set_sim900_emu(NULL);
  ysim900_emu_free(emu);
  yserial_free(serial);
  yparser_free(parser);
}

/*
Scenario: after power up it should not be allowed to send any other command than
ATE0
*/

void ysim900_emu_test_suite() {
  should_allow_to_power_on_modem();
  should_allow_to_process_when_modem_is_powered_off();
  should_not_allow_to_power_on_modem_if_pulse_is_too_short();
  should_allow_ate0_command_after_power_up();
}

#endif
