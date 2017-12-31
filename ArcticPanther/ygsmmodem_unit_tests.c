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

#ifdef RUN_YGSMMODEM_UNIT_TESTS

#include "yassert.h"
#include "ygsmmodem.h"
#include "yserial.h"
#include "ytimer.h"

#include "ygsmmodem.h"
#include "ysim900_emulator.h"

#include <memory.h>
#include <stdbool.h>
#include <stdlib.h>

#define POWER_ON_TIME (120000)
#define A_HELL_LOT_MORE_TIME (240000)

/*
Story A: Control power
*/

bool callback_was_called = false;

static void wake_success_cb_story_a_scenario_1(yerrno_t result) {
  callback_was_called = true;
  yassert(result == Y_OK);
}

static void spend_time_waiting_for_callback(uint32_t *current_time_ms,
                                            uint32_t waiting_time_ms,
                                            ysim900_emu_t *emu) {
  while (!callback_was_called && *current_time_ms < waiting_time_ms) {
    yassert(*current_time_ms == ytimer_get_uptime_in_ms() + 1);

    ymodem_tick();
    ysim900_emu_tick(*emu);
    ytimer_tick((*current_time_ms)++);
  }
}

/*
Scenario 1: It should be possible for user to power on the modem
*/
static void
should_be_possible_for_user_to_power_on_the_modem_if_modem_is_off() {
  // given power in the modem is disabled
  uint32_t current_time = 0;
  ytimer_set_uptime_in_ms(current_time++);
  ytimer_force_init();

  yserial_t serial = yserial_alloc(19200, 7, 8);
  ysim900_emu_t emu = ysim900_emu_alloc(serial);
  yhal_set_sim900_emu(emu);

  ysim900_emu_set_is_on(emu, false);
  ysim900_emu_set_power_control_pin(emu, Y_LOW);

  ymodem_connect(serial, NULL, NULL, NULL, NULL);

  int8_t is_powered_on = 42;
  int8_t signal_strength = 42;

  // when modem is asked for status
  callback_was_called = false;
  ymodem_set_is_enabled(true, wake_success_cb_story_a_scenario_1);

  ymodem_get_status(&is_powered_on, &signal_strength);
  yassert(is_powered_on == 0);
  yassert(callback_was_called == false);
  yassert(ymodem_is_busy() == true);

  spend_time_waiting_for_callback(&current_time, POWER_ON_TIME, &emu);

  // rest of the test is moven to the callback
  // wake_success_cb_story_a_scenario_1
  ymodem_get_status(&is_powered_on, &signal_strength);

  // then it should tell it is not powered on
  yassert(is_powered_on == true);
  yassert(callback_was_called == true);
  yassert(ymodem_is_busy() == false);

  ymodem_disconnect();
  ysim900_emu_free(emu);
  yserial_free(serial);
}

static void should_be_possible_for_user_to_power_on_the_modem_if_modem_is_on() {
  // given power in the modem is disabled
  int current_time = 0;
  ytimer_set_uptime_in_ms(current_time++);
  ytimer_force_init();

  yserial_t serial = yserial_alloc(19200, 7, 8);
  ysim900_emu_t emu = ysim900_emu_alloc(serial);
  yhal_set_sim900_emu(emu);

  ysim900_emu_set_is_on(emu, true);
  ysim900_emu_set_power_control_pin(emu, Y_LOW);

  ymodem_connect(serial, NULL, NULL, NULL, NULL);

  int8_t is_powered_on = 42;
  int8_t signal_strength = 42;

  // when modem is asked for status
  callback_was_called = false;

  ymodem_get_status(&is_powered_on, &signal_strength);
  // yassert(is_powered_on == 1); // TODO
  yassert(callback_was_called == false);
  yassert(ymodem_is_busy() == false);

  ymodem_set_is_enabled(true, wake_success_cb_story_a_scenario_1);

  spend_time_waiting_for_callback(&current_time, POWER_ON_TIME, &emu);

  // rest of the test is moven to the callback
  // wake_success_cb_story_a_scenario_1
  ymodem_get_status(&is_powered_on, &signal_strength);

  // then it should tell it is not powered on
  yassert(is_powered_on == 1);
  yassert(callback_was_called == true);
  yassert(ymodem_is_busy() == false);

  ymodem_disconnect();
  ysim900_emu_free(emu);
  yserial_free(serial);
}

/*
Scenario 2: It should be possible for user to power off the modem
Given:
        Modem is powered on
When:
        Asked to power off the modem
Then:
        Modem is being powered off and it is measurable trough it's status
*/
static void
should_be_possible_for_user_to_power_off_the_modem_if_modem_is_off() {
  // given power in the modem is disabled
  int current_time = 0;
  ytimer_set_uptime_in_ms(current_time++);
  ytimer_force_init();

  yserial_t serial = yserial_alloc(19200, 7, 8);
  ysim900_emu_t emu = ysim900_emu_alloc(serial);
  yhal_set_sim900_emu(emu);

  ysim900_emu_set_is_on(emu, false);
  ysim900_emu_set_power_control_pin(emu, Y_LOW);

  ymodem_connect(serial, NULL, NULL, NULL, NULL);

  bool is_powered_on = false;
  int8_t signal_strength = 42;

  // when modem is asked for status
  callback_was_called = false;
  ymodem_get_status(&is_powered_on, &signal_strength);
  yassert(is_powered_on == 0);
  yassert(callback_was_called == false);
  ymodem_set_is_enabled(false, wake_success_cb_story_a_scenario_1);
  spend_time_waiting_for_callback(&current_time, POWER_ON_TIME, &emu);

  callback_was_called = false;
  ymodem_set_is_enabled(false, wake_success_cb_story_a_scenario_1);
  spend_time_waiting_for_callback(&current_time, A_HELL_LOT_MORE_TIME, &emu);
  // rest of the test is moven to the callback
  // wake_success_cb_story_a_scenario_1
  ymodem_get_status(&is_powered_on, &signal_strength);

  // then it should tell it is not powered on
  yassert(is_powered_on == false);
  yassert(callback_was_called == true);
  yassert(ymodem_is_busy() == false);

  ymodem_disconnect();
  ysim900_emu_free(emu);
  yserial_free(serial);
}

static void
should_be_possible_for_user_to_power_off_the_modem_if_modem_is_on() {
  // given power in the modem is enabled
  int current_time = 0;
  ytimer_set_uptime_in_ms(current_time++);
  ytimer_force_init();

  yserial_t serial = yserial_alloc(19200, 7, 8);
  ysim900_emu_t emu = ysim900_emu_alloc(serial);
  yhal_set_sim900_emu(emu);

  ysim900_emu_set_is_on(emu, true);
  ysim900_emu_set_power_control_pin(emu, Y_LOW);

  ymodem_connect(serial, NULL, NULL, NULL, NULL);

  bool is_powered_on = false;
  int8_t signal_strength = 42;

  callback_was_called = false;
  ymodem_set_is_enabled(true, wake_success_cb_story_a_scenario_1);

  spend_time_waiting_for_callback(&current_time, POWER_ON_TIME, &emu);

  ymodem_get_status(&is_powered_on, &signal_strength);
  yassert(is_powered_on);
  yassert(ymodem_is_busy() == false);

  // when modem is asked for status

  callback_was_called = false;
  ymodem_set_is_enabled(false, wake_success_cb_story_a_scenario_1);

  spend_time_waiting_for_callback(&current_time, A_HELL_LOT_MORE_TIME, &emu);

  // rest of the test is moven to the callback
  // wake_success_cb_story_a_scenario_1
  ymodem_get_status(&is_powered_on, &signal_strength);

  // then it should tell it is not powered on
  yassert(is_powered_on == false);
  yassert(callback_was_called == true);
  yassert(ymodem_is_busy() == false);

  ymodem_disconnect();
  ysim900_emu_free(emu);
  yserial_free(serial);
}

/**
Given:
        Modem was powered on
        Timeout sufficient to power down the modem passed
When:
        User asks for the power status
Then:
        Power is reported to be on
*/
static void
should_be_possible_for_user_to_verify_modem_is_turned_off_after_timeout_passes() {
  // given power in the modem is enabled
  int current_time = 0;
  ytimer_set_uptime_in_ms(current_time++);
  ytimer_force_init();

  yserial_t serial = yserial_alloc(19200, 7, 8);
  ysim900_emu_t emu = ysim900_emu_alloc(serial);
  yhal_set_sim900_emu(emu);

  ysim900_emu_set_is_on(emu, true);
  ysim900_emu_set_power_control_pin(emu, Y_LOW);

  ymodem_connect(serial, NULL, NULL, NULL, NULL);

  bool is_powered_on = false;
  int8_t signal_strength = 42;

  callback_was_called = false;
  ymodem_set_is_enabled(true, wake_success_cb_story_a_scenario_1);

  spend_time_waiting_for_callback(&current_time, POWER_ON_TIME, &emu);

  ymodem_get_status(&is_powered_on, &signal_strength);
  yassert(is_powered_on == true);
  yassert(ymodem_is_busy() == false);

  // time passes
  while (current_time < MODEM_INACTIVITY_TIMEOUT * 30) {
    yassert(current_time == ytimer_get_uptime_in_ms() + 1);
    if (current_time % 100000 == 0) {
      ytracestr("Some time passed");
    }

    ymodem_tick();
    ysim900_emu_tick(emu);
    ytimer_tick(current_time++);
  }

  // when modem is asked for status then the modem should be turned off
  ymodem_get_status(&is_powered_on, &signal_strength);
  yassert(is_powered_on == false);
  yassert(ymodem_is_busy() == false);
  yassert(signal_strength == 0);

  ymodem_disconnect();
  ysim900_emu_free(emu);
  yserial_free(serial);
}

/*
Story B: Check the status

Scenario 1: It should be possible for user to determine if modem is poweded on
Scenario 2: It should be possible for user to determine if modem is connected to
a network Scenario 3: It should be possible for user to determine signal
strength
*/

/*
Story C: Setup network parameters
*/

/*
Story D: Connect to a network
*/

/*
Story E: Send SMS
*/

/*
Story F: Receive SMS
*/

/**
Story: Send HTTP request and receive response

Given:
        Modem was powered off
        Timeout sufficient to power down the modem passed
        Modem is not busy
When:
        User asks to get google.com
Then:
    google.com content is retreived
*/

bool http_callback_was_called = false;

static void http_get_cb_success(yerrno_t result, uint8_t *buff, uint8_t len) {
  int i = 0;
  http_callback_was_called = true;
  yassert(result == Y_OK);
  yassert(len == 128);
  for (i = 0; i < len; i++) {
    yassert(buff[i] == i + 1);
  }
}

static void
should_be_possible_for_user_to_reqtrieve_google_page_when_modem_is_off() {
  // given
  uint32_t current_time = 0;
  uint8_t buff[128];

  ytimer_set_uptime_in_ms(current_time++);
  ytimer_force_init();

  yserial_t serial = yserial_alloc(19200, 7, 8);
  ysim900_emu_t emu = ysim900_emu_alloc(serial);
  yhal_set_sim900_emu(emu);

  ysim900_emu_set_is_on(emu, false);
  ysim900_emu_set_power_control_pin(emu, Y_LOW);

  ymodem_connect(serial, NULL, NULL, NULL, NULL);
  ymodem_set_is_enabled(false, wake_success_cb_story_a_scenario_1);
  spend_time_waiting_for_callback(&current_time, POWER_ON_TIME, &emu);

  callback_was_called = http_callback_was_called = false;

  // when
  ymodem_http_get("www.google.com", http_get_cb_success, buff, 128);
  spend_time_waiting_for_callback(&current_time, A_HELL_LOT_MORE_TIME, &emu);

  // then
  yassert(http_callback_was_called == true);

  ymodem_disconnect();
  ysim900_emu_free(emu);
  yserial_free(serial);
}

/**
Story: Send HTTP request and receive response

Given:
Modem was powered on
Timeout sufficient to power down the modem passed
Modem is not busy
When:
User asks to get google.com
Then:
google.com content is retreived
*/
static void
should_be_possible_for_user_to_send_http_request_when_modem_is_on() {
  // given
  uint32_t current_time = 0;
  uint8_t buff[128];

  ytimer_set_uptime_in_ms(current_time++);
  ytimer_force_init();

  yserial_t serial = yserial_alloc(19200, 7, 8);
  ysim900_emu_t emu = ysim900_emu_alloc(serial);
  yhal_set_sim900_emu(emu);

  ysim900_emu_set_is_on(emu, true);
  ysim900_emu_set_power_control_pin(emu, Y_LOW);

  ymodem_connect(serial, NULL, NULL, NULL, NULL);

  ymodem_set_is_enabled(true, wake_success_cb_story_a_scenario_1);
  spend_time_waiting_for_callback(&current_time, POWER_ON_TIME, &emu);

  callback_was_called = http_callback_was_called = false;

  // when
  ymodem_http_get("www.google.com", http_get_cb_success, buff, 128);

  spend_time_waiting_for_callback(&current_time, A_HELL_LOT_MORE_TIME, &emu);

  // then
  yassert(http_callback_was_called == true);

  ymodem_disconnect();
  ysim900_emu_free(emu);
  yserial_free(serial);
}

void ygsmmodem_execute_unit_tests() {
#ifdef _WIN32
  int i;

  for (i = 0; i < 3; i++) {
#endif
    should_be_possible_for_user_to_power_on_the_modem_if_modem_is_off();
    should_be_possible_for_user_to_power_on_the_modem_if_modem_is_on();
    should_be_possible_for_user_to_power_off_the_modem_if_modem_is_off();
    should_be_possible_for_user_to_power_off_the_modem_if_modem_is_on();
    should_be_possible_for_user_to_verify_modem_is_turned_off_after_timeout_passes();
    should_be_possible_for_user_to_reqtrieve_google_page_when_modem_is_off();
    should_be_possible_for_user_to_send_http_request_when_modem_is_on();
#ifdef _WIN32
  }
#endif
}

#endif
