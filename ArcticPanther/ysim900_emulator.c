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
#include "ycollection.h"
#include "yparser.h"
#include "ysim900_emulator.h"
#include "ystring.h"
#include "ytimer.h"

#define POWER_PIN_HIGH_PRIMING_MS (2000)
#define POWER_PIN_LOW_PRIMING_MS (1000)

struct ysim900_emu {
  yserial_t serial;
  bool did_fatal_error_occured;
  bool is_on;
  bool explode_on_error;

  pin_state_t power_pin_state;
  uint64_t when_pin_was_toggled;
  uint8_t power_priming_state;
  ycollection_t incoming_bytes;
  yparser_t parser;
};

/*
Scenario
*/

static void explode(ysim900_emu_t emu) { yassert(false); }

void receiver_cb(yserial_t serial, void *cookie, const uint8_t *buff,
                 size_t buff_len) {
  ysim900_emu_t emu = (ysim900_emu_t)cookie;
  size_t i;

  if (emu->is_on == false) {
    // NOP
  } else {
    for (i = 0; i < buff_len; i++) {
      ycollection_push(emu->incoming_bytes, (void *)buff[i]);
    }
  }
}

ysim900_emu_t ysim900_emu_alloc(yserial_t serial) {
  ysim900_emu_t result = malloc(sizeof(struct ysim900_emu));
  yassert(result != NULL);
  yassert(serial != NULL);
  result->serial = serial;
  result->did_fatal_error_occured = false;
  result->explode_on_error = true;
  result->is_on = false;
  result->power_priming_state = 0; // 0 is not primed
  result->when_pin_was_toggled = ytimer_get_uptime_in_ms();
  result->power_pin_state = Y_LOW;
  result->incoming_bytes = ycollection_list_alloc(sizeof(uint8_t), NULL);
  result->parser = yparser_alloc(FSTRING_COUNT, 0, ystring_fstring_getter);

  yassert(result->parser != NULL);
  yassert(result->incoming_bytes != NULL);

  yserial_test_set_fake_mode(serial, receiver_cb, (void *)result);

  return result;
}

yerrno_t ysim900_emu_set_is_on(ysim900_emu_t emu, bool is_on) {
  emu->is_on = is_on;
  return Y_OK;
}

yerrno_t ysim900_emu_set_power_control_pin(ysim900_emu_t emu,
                                           pin_state_t state) {
  ytrace(state);
  if (state != emu->power_pin_state) {
    emu->power_pin_state = state;
    emu->when_pin_was_toggled = ytimer_get_uptime_in_ms();
  } else {
    if (ytimer_get_uptime_in_ms() < emu->when_pin_was_toggled) {
      emu->when_pin_was_toggled = ytimer_get_uptime_in_ms();
    }
  }
  return Y_OK;
}

static void send_ok(ysim900_emu_t emu) {
  int b;
  int i = 0;
  while ((b = ystring_get_char_from_fstring(ATT_OK, i)) != -1) {
    uint8_t casted_b = b;
    yserial_test_simulate_received(emu->serial, &casted_b, sizeof(uint8_t));
    i++;
  }
}

void ysim900_emu_tick(ysim900_emu_t emu) {
  uint64_t t = ytimer_get_uptime_in_ms();
  if (emu->is_on == false) {
    if (emu->power_pin_state == Y_HIGH &&
        (t - emu->when_pin_was_toggled) >= POWER_PIN_HIGH_PRIMING_MS - 1) {
      emu->power_priming_state = 1; // waiting for low
    }
    if (emu->power_pin_state == Y_LOW &&
        (t - emu->when_pin_was_toggled) >= POWER_PIN_LOW_PRIMING_MS - 1 &&
        emu->power_priming_state == 1) {
      emu->is_on = true;
      emu->power_priming_state = 0;
      yparser_reset(emu->parser);
      yserial_test_simulate_received(emu->serial, (const uint8_t *)emu,
                                     sizeof(struct ysim900_emu));
      send_ok(emu);
    }
  } else {
    while (ycollection_size(emu->incoming_bytes) != 0) {
      uint8_t b;
      int command_idx;
      static uint8_t randomizer = 0;

      ycollection_get(emu->incoming_bytes, 0, &b);
      ycollection_remove(emu->incoming_bytes, 0);

      command_idx = yparser_next(emu->parser, b);

      if (command_idx == YPARSER_RESULT_NO_MATCH_FOUND) {
        yparser_reset(emu->parser);
        continue;
      } else if (command_idx == YPARSER_RESULT_MORE_DATA_REQUIRED) {
        continue;
      } else if (command_idx < FSTRING_COUNT) {
        int i = 0;
        yparser_reset(emu->parser);

        if (command_idx == ATT_ATE0) {
          yserial_test_simulate_received(emu->serial, "ATE0\r\n", 6);
          send_ok(emu);
        } else if (command_idx == ATT_COPS) {
          yserial_test_simulate_received(emu->serial, "+COPS: 0,0,\"PLAY\"\r\n",
                                         25); // TODO
          send_ok(emu);
        } else if (command_idx == ATT_CSQ) {
          static uint8_t success = 0;

          if (success % 3 == 0) {
            yserial_test_simulate_received(emu->serial, "+CSQ: 12,0\r\n", 12);
          } else {
            yserial_test_simulate_received(emu->serial, "+CSQ: 0,0\r\n", 11);
          }
          success++;
          send_ok(emu);
        } else if (command_idx == ATT_CREG) {
          if (randomizer % 3 == 1) {
            yserial_test_simulate_received(emu->serial, "+CREG: 0,5\r\n", 12);
          } else {
            yserial_test_simulate_received(emu->serial, "+CREG: 0,0\r\n", 12);
          }
          randomizer++;
          send_ok(emu);
        } else if (command_idx == ATT_HTTP_ACTION_0) {
          const static char *HTTP_ACTION_RESPONSE = "+HTTPACTION:0,200,3\r\n";
          if (randomizer % 2 == 1) {
            send_ok(emu);
            yserial_test_simulate_received(
                emu->serial, HTTP_ACTION_RESPONSE,
                ystring_strlen(HTTP_ACTION_RESPONSE));
          } else {
            yserial_test_simulate_received(
                emu->serial, HTTP_ACTION_RESPONSE,
                ystring_strlen(HTTP_ACTION_RESPONSE));
            send_ok(emu);
          }
          randomizer++;
        } else if (command_idx == ATT_HTTP_READ) {
          // send_ok(emu);
          const static char *HTTP_READ_RESPONSE = "+HTTPREAD:128\r\n";
          uint8_t i = 0;

          yserial_test_simulate_received(emu->serial, HTTP_READ_RESPONSE,
                                         ystring_strlen(HTTP_READ_RESPONSE));

          for (i = 1; i < 129; i++) {
            yserial_test_simulate_received(emu->serial, (char *)&i, 1);
          }
          yserial_test_simulate_received(emu->serial, "\r\n", 2);
          send_ok(emu);
        } else if (command_idx == ATT_CGATT_1 || command_idx == ATT_AT ||
                   command_idx == ATT_CEER_0 || command_idx == ATT_CREG_0 ||
                   command_idx == ATT_CGDCONT_1 ||
                   command_idx == ATT_SAPBR_CONTYPE ||
                   command_idx == ATT_SAPBR_APN ||
                   command_idx == ATT_SAPBR_USER ||
                   command_idx == ATT_SAPBR_PASSWD ||
                   command_idx == ATT_SAPBR_SET ||
                   command_idx == ATT_HTTPINIT ||
                   command_idx == ATT_HTTPARA_URL_PREFIX ||
                   command_idx == ATT_HTTP_TERM || command_idx == ATT_CMEE_2 ||
                   command_idx == ATT_HTTPPARA_SET_CONTEXT_ID) {
          send_ok(emu);
        }
      }
    }
  }
}

yerrno_t ysim900_explode_on_error(ysim900_emu_t emu, bool should_explode) {
  emu->explode_on_error = should_explode;
  return Y_OK;
}

yerrno_t ysim900_emu_free(ysim900_emu_t emu) {
  yparser_free(emu->parser);
  ycollection_free(emu->incoming_bytes);
  free(emu);
  return Y_OK;
}

bool ysim900_exploded(ysim900_emu_t emu) {
  return emu->did_fatal_error_occured;
}

#endif
