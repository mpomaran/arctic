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

/*

3 Jan 2017 - new design

The GSM networking implementation depends on conecpt of layers:
 - power (flow) control
 - At commands
 - etc.

Each command is composed from series of actions and each action can be performed
by various layers, however in the end there is only one input - serial.

Main concepts:
- higher level layer can invoke lower level layer, result is passed by calling
callback
- each command can set up it's own consumer which consumes and unserstands only
syntax applicable for itself

Uses protothreads (http://dunkels.com/adam/pt/examples.html#driver) based on
coroutines http://dunkels.com/adam/dunkels06protothreads.pdf


*/

#include "ygsmmodem.h"
#include "yassert.h"
#include "yparser.h"
#include "yserial.h"
#include "ystring.h"
#include "ytimer.h"

#include <pt.h>
#include <stdlib.h>

#ifndef NULL
#error
#endif

#define YMODEM_EVENT_LOOP_SIZE (16)
#define CR (13)
#define LF (10)
static const char *CRLF = "\r\n";

#define WAIT_FOR_NETOWRK_TIMEOUT (5000)
#define WAIT_FOR_NETWORK_MAX_COUNT (25)
#define MIN_ACCEPTABLE_SIGNAL_STRENGTH (10)
#define MAX_ACCEPTABLE_BIT_ERROR_RATE (7)
#define REGISTERED_TO_HOME_NETWORK (1)
#define REGISTERED_TO_ROADMING_NETWORK (5)
#define SERIAL_BUFFER_SIZE (32)

#define EXIT_IF_ERROR(P)                                                       \
  do {                                                                         \
    if (enabler.error == true)                                                 \
      PT_EXIT(P);                                                              \
  } while (0)

/* for more info what this code does go here:
 * http://tronixstuff.com/2014/01/08/tutorial-arduino-and-sim900-gsm-modules/ */
static struct {
  struct pt pt;

  enum { DISABLED, ENABLING, ENABLED, READY_TO_TRANSMIT, DISABLING } state;

  const char *apn_name;
  const char *ip;
  const char *user;
  const char *password;

  uint8_t signal_strength;

  const char *url;
  uint8_t *msg_buff;
  uint8_t msg_buff_len;
  uint8_t msg_bytes_read;
  ymodem_http_get_callback_fn http_callback;

  bool inactivity_timer_expired;
  uint8_t inactivity_timer_id;

  ymodem_callback_fn result_callback;

  yerrno_t error;
} modem;

/* ****************************************************** */
/* Serial state machine                                   */
/* ****************************************************** */
typedef enum { LINE, BYTE } serial_consumer_mode_t;

static struct {
  struct pt pt;
  serial_consumer_mode_t mode;
  yserial_t serial;
  union {
    uint8_t line[SERIAL_BUFFER_SIZE];
    uint8_t byte;
  };
  uint8_t idx;

  bool buffer_full;
} serial_consumer;

static bool is_serial_ready() { return serial_consumer.buffer_full; }

static void clear_serial_buffer() {
  serial_consumer.idx = 0;
  serial_consumer.buffer_full = false;
  ystring_memzero(serial_consumer.line, SERIAL_BUFFER_SIZE);
}

static uint8_t get_serial_byte() {
  clear_serial_buffer();
  return serial_consumer.byte;
}

// bytes will vanish when the serial state machine is called again
static uint8_t *get_line(uint8_t *len) {
  if (len != NULL) {
    *len = serial_consumer.idx;
  }
  return serial_consumer.line;
}

static PT_THREAD(serial_consumer_fiber(struct pt *pt)) {
  PT_BEGIN(pt);

  while (1) {
    do {
      /* read idle */
      PT_WAIT_UNTIL(pt, (yserial_available(serial_consumer.serial) &&
                         serial_consumer.buffer_full == false) ||
                            serial_consumer.mode == BYTE);
      {
        uint8_t c;

        if (serial_consumer.mode == BYTE) {
          break;
        }

        c = yserial_read_byte(serial_consumer.serial);

        // if (c == 1) {
        //	ytrace(c);
        //}
        if (c == CR || c == 0 || c == 255) {
          // NOP
        } else if (c == LF) {
          if (serial_consumer.idx == 0) {
            // NOP
          } else {
            //						ytracestr(serial_consumer.line);
            serial_consumer.buffer_full = true;
          }
        } else {
          //					ytrace(c);
          serial_consumer.line[serial_consumer.idx++] = c;
          if (serial_consumer.idx == SERIAL_BUFFER_SIZE) {
            serial_consumer.buffer_full = true;
          }
        }
      }
    } while (serial_consumer.mode == LINE);

    /* read byte */
    do {
      PT_WAIT_UNTIL(pt, (yserial_available(serial_consumer.serial) &&
                         serial_consumer.buffer_full == false) ||
                            serial_consumer.mode == LINE);

      serial_consumer.byte = yserial_read_byte(serial_consumer.serial);
      // if (serial_consumer.byte == 1) {
      //	ytrace(serial_consumer.byte);
      //}

      serial_consumer.buffer_full = true;
    } while (serial_consumer.mode == BYTE);
  }

  PT_END(pt);
}

static void set_serial_consumer_mode(serial_consumer_mode_t mode) {
  serial_consumer.mode = mode;
}

static PT_THREAD(serial_write(const uint8_t *str, struct pt *pt)) {
  static uint8_t *str_ptr;

  PT_BEGIN(pt);

  str_ptr = (uint8_t *)str;
  while (*str_ptr != 0) {
    yserial_write_data(serial_consumer.serial, str_ptr, 1);
    str_ptr++;
    PT_YIELD(pt);
  }

  PT_END(pt);
}

/* ****************************************************** */
/* Power cycle state machine                              */
/* ****************************************************** */
static struct {
  bool timeout_expired;
  struct pt pt;
} pwr_cycler;

extern void hardware_power_up_cb(void *cookie) {
  pwr_cycler.timeout_expired = true;
}

static PT_THREAD(power_cycle(struct pt *pt)) {
  PT_BEGIN(pt);

  /* power cycle modem */
  yhal_set_pin_mode(SIM900_POWER_PIN, Y_PIN_MODE_OUTPUT);
  yhal_set_pin_state(SIM900_POWER_PIN, Y_LOW);
  pwr_cycler.timeout_expired = false;
  ytimer_setup_onetime_callback(hardware_power_up_cb, 1000);
  PT_WAIT_UNTIL(pt, pwr_cycler.timeout_expired);

  yhal_set_pin_state(SIM900_POWER_PIN, Y_HIGH);
  pwr_cycler.timeout_expired = false;
  ytimer_setup_onetime_callback(hardware_power_up_cb, 2000);
  PT_WAIT_UNTIL(pt, pwr_cycler.timeout_expired);

  yhal_set_pin_state(SIM900_POWER_PIN, Y_LOW);
  pwr_cycler.timeout_expired = false;
  ytimer_setup_onetime_callback(hardware_power_up_cb, 30000);

  PT_WAIT_UNTIL(pt, pwr_cycler.timeout_expired);

  /* clear serial */
  while (yserial_available(serial_consumer.serial)) {
    yserial_read_byte(serial_consumer.serial);
  }
  clear_serial_buffer();

  PT_END(pt);
}

/* ****************************************************** */
/* AT protocol fiber                                      */
/* ****************************************************** */
#define AT_ACK_TIMEOUT (5000)
#define HTTP_TIMEOT (30000)
static struct {
  struct pt pt;
  struct pt serial_write_pt;
  struct pt ok_pt;
  struct pt read_param_pt;
  struct pt wait_for_line_at;

  bool error;
  bool timeout_expired;
  uint8_t timeout_id;

  union {
    uint16_t params[3];
    struct {
      uint16_t param1, param2, param3;
    };
  } params;
  uint8_t param_count;
} at;

extern void at_cb(void *cookie) {
  at.timeout_expired = true;
  // ytrace(9);
}

/**
Calee is responsible for clearing serial buffer after this call
*/
static PT_THREAD(at_wait_for_line(struct pt *pt, uint32_t timeout)) {
  PT_BEGIN(pt);

  /* setup timeout */
  at.timeout_expired = false;
  at.timeout_id = ytimer_setup_onetime_callback(at_cb, timeout);
  // ytrace(10);
  // ytrace(timeout);

  /* wait until timeout expires or OK is received */
  PT_WAIT_UNTIL(pt, is_serial_ready() || at.timeout_expired);
  if (at.timeout_expired) {
    at.error = true;
    // ytrace(11);
  } else {
    ytimer_reset_onetime_callback(at.timeout_id);
    // ytrace(12);

    /* serial ready */
    yassert(is_serial_ready());
    at.error = false;
  }

  PT_END(pt);
}

static PT_THREAD(at_wait_for_ok(struct pt *pt)) {
  uint8_t *line;
  uint8_t len;

  PT_BEGIN(pt);

  PT_SPAWN(pt, &at.wait_for_line_at,
           at_wait_for_line(&at.wait_for_line_at, AT_ACK_TIMEOUT));
  if (at.error) {
    clear_serial_buffer();
    PT_EXIT(pt);
  }

  line = get_line(&len);
  line[len] = 0;

  ystring_cpy_fstring_to_buff(ATT_ATE0);
  if (ystring_equals(ystring_buff, line, len)) {
    clear_serial_buffer();
    PT_SPAWN(pt, &at.wait_for_line_at,
             at_wait_for_line(&at.wait_for_line_at, AT_ACK_TIMEOUT));
    if (at.error) {
      clear_serial_buffer();
      PT_EXIT(pt);
    }
    line = get_line(&len);
    line[len] = 0;
  }

  ystring_cpy_fstring_to_buff(ATT_OK);
  at.error = !ystring_equals(ystring_buff, line, len);

#ifdef ARDUINO_ARCH_AVR
  ytracestr(line);
#endif

  clear_serial_buffer();

  PT_END(pt);
}

static PT_THREAD(at_set(const char *cmd_str, struct pt *pt)) {
  PT_BEGIN(pt);

  /* setup serial communication*/
  at.error = false;
  set_serial_consumer_mode(LINE);
  clear_serial_buffer();
  yassert(serial_consumer.idx == 0);

  /* send command in a new fiber */
  PT_SPAWN(pt, &at.serial_write_pt, serial_write(cmd_str, &at.serial_write_pt));
  PT_SPAWN(pt, &at.serial_write_pt, serial_write(CRLF, &at.serial_write_pt));

  PT_SPAWN(pt, &at.ok_pt, at_wait_for_ok(&at.ok_pt));

  PT_END(pt);
}

static yerrno_t parse_at_response(const char *line, uint8_t len) {
  /* read params */
  int16_t idx;
  at.param_count = 0;

  idx = ystring_get_char_idx(line, len, ':');

  if (idx < 0) {
    return Y_ERROR;
  }
  idx += 1;

  /* now read numbers */
  do {
    int16_t tmp;
    at.params.params[at.param_count++] =
        ystring_parse_int(line + idx, len - idx);
    tmp = ystring_get_char_idx(line + idx, len - idx, ',');
    if (tmp < 0) {
      break;
    }
    idx += tmp + 1;
  } while (idx > 0 && at.param_count < 3);

  return Y_OK;
}

static PT_THREAD(at_read_params(struct pt *pt)) {
  PT_BEGIN(pt);

  /* setup serial communication*/
  at.error = false;
  set_serial_consumer_mode(LINE);

  /* setup timeout */
  at.timeout_expired = false;
  at.timeout_id = ytimer_setup_onetime_callback(at_cb, AT_ACK_TIMEOUT);

  /* wait until timeout expires or OK is received */
  PT_WAIT_UNTIL(pt, is_serial_ready() || at.timeout_expired);
  if (at.timeout_expired) {
    at.error = true;
  } else {
    /* read params */
    uint8_t *line;
    uint8_t len;

    ytimer_reset_onetime_callback(at.timeout_id);

    yassert(is_serial_ready());
    line = get_line(&len);
    at.error = parse_at_response(line, len) == Y_ERROR ? true : false;
    clear_serial_buffer();
  }

  PT_END(pt);
}

static PT_THREAD(at_read(const char *cmd_str, struct pt *pt)) {
  PT_BEGIN(pt);

  /* send command in a new fiber */
  clear_serial_buffer();
  PT_SPAWN(pt, &at.serial_write_pt, serial_write(cmd_str, &at.serial_write_pt));
  PT_SPAWN(pt, &at.serial_write_pt, serial_write(CRLF, &at.serial_write_pt));

  /* wait for read result */
  PT_SPAWN(pt, &at.read_param_pt, at_read_params(&at.read_param_pt));

  if (at.error) {
    PT_EXIT(pt);
  }

  /* wait for final ok */
  PT_SPAWN(pt, &at.ok_pt, at_wait_for_ok(&at.ok_pt));

  PT_END(pt);
}

/* ****************************************************** */
/* Enabling state machine                                 */
/* ****************************************************** */
static struct {
  struct pt pt;
  struct pt at_pt;

  bool error;
  bool timeout_expired;
  bool power_off_on_error;
  uint8_t try_count;
  timer_id_t timeout_id;
} enabler;

extern void wait_for_network_cb(void *cookie) {
  enabler.timeout_expired = true;
}

static PT_THREAD(enablng_fiber(struct pt *pt)) {
  PT_BEGIN(pt);

  enabler.error = true;
  enabler.power_off_on_error = false;

  /* power discovery */
  ystring_cpy_fstring_to_buff(ATT_ATE0);
  PT_SPAWN(pt, &enabler.at_pt, at_set(ystring_buff, &enabler.at_pt));

  if (at.error) {
    /* power cycle */
    PT_SPAWN(pt, &pwr_cycler.pt, power_cycle(&pwr_cycler.pt));

    /* last try to get OK */
    ystring_cpy_fstring_to_buff(ATT_ATE0);
    PT_SPAWN_AND_ASSERT_SUCCESS(pt, &enabler.at_pt,
                                at_set(ystring_buff, &enabler.at_pt),
                                at.error == false);
  }

  /* let calee know if power should be disabled if error occurs */
  enabler.power_off_on_error = true;

  /* power is on, now set the modem */
  ystring_cpy_fstring_to_buff(ATT_CEER_0);
  PT_SPAWN_AND_ASSERT_SUCCESS(pt, &enabler.at_pt,
                              at_set(ystring_buff, &enabler.at_pt),
                              at.error == false);

  ystring_cpy_fstring_to_buff(ATT_CREG_0);
  PT_SPAWN_AND_ASSERT_SUCCESS(pt, &enabler.at_pt,
                              at_set(ystring_buff, &enabler.at_pt),
                              at.error == false);

  /* wait for network registration and a good signal*/
  enabler.try_count = 0;

  do {
    ystring_cpy_fstring_to_buff(ATT_CREG);
    PT_SPAWN_AND_ASSERT_SUCCESS(pt, &enabler.at_pt,
                                at_read(ystring_buff, &enabler.at_pt),
                                at.error == false);
    if (at.param_count >= 2 &&
        (at.params.param2 == REGISTERED_TO_HOME_NETWORK ||
         at.params.param2 == REGISTERED_TO_ROADMING_NETWORK)) {
      // if registered to the network
      break;
    }

    if (enabler.try_count++ >= WAIT_FOR_NETWORK_MAX_COUNT) {
      enabler.error = true;
      PT_EXIT(pt);
    } else {
      enabler.timeout_expired = false;
      ytimer_setup_onetime_callback(wait_for_network_cb,
                                    WAIT_FOR_NETOWRK_TIMEOUT);
      PT_WAIT_UNTIL(pt, enabler.timeout_expired);
    }
  } while (1);

  /* wait for good signal*/
  enabler.try_count = 0;

  do {
    ystring_cpy_fstring_to_buff(ATT_CSQ);
    PT_SPAWN_AND_ASSERT_SUCCESS(pt, &enabler.at_pt,
                                at_read(ystring_buff, &enabler.at_pt),
                                at.error == false);

    modem.signal_strength = (uint8_t)at.params.params[0];

    if (modem.signal_strength > MIN_ACCEPTABLE_SIGNAL_STRENGTH &&
        at.params.params[1] < MAX_ACCEPTABLE_BIT_ERROR_RATE) {
      break;
    }

    if (enabler.try_count++ >= WAIT_FOR_NETWORK_MAX_COUNT) {
      enabler.error = true;
      PT_EXIT(pt);
    } else {
      enabler.timeout_expired = false;
      ytimer_setup_onetime_callback(wait_for_network_cb,
                                    WAIT_FOR_NETOWRK_TIMEOUT);
      PT_WAIT_UNTIL(pt, enabler.timeout_expired);
    }
  } while (1);

  /* setup gprs communication */

  /* TODO provider discovery */
  ystring_cpy_fstring_to_buff(ATT_CGATT_1);
  PT_SPAWN_AND_ASSERT_SUCCESS(pt, &enabler.at_pt,
                              at_set(ystring_buff, &enabler.at_pt),
                              at.error == false);

  ystring_cpy_fstring_to_buff(ATT_SAPBR_CONTYPE);
  PT_SPAWN_AND_ASSERT_SUCCESS(pt, &enabler.at_pt,
                              at_set(ystring_buff, &enabler.at_pt),
                              at.error == false);

  ystring_cpy_fstring_to_buff(ATT_SAPBR_APN);
  PT_SPAWN_AND_ASSERT_SUCCESS(pt, &enabler.at_pt,
                              at_set(ystring_buff, &enabler.at_pt),
                              at.error == false);

  ystring_cpy_fstring_to_buff(ATT_SAPBR_USER);
  PT_SPAWN_AND_ASSERT_SUCCESS(pt, &enabler.at_pt,
                              at_set(ystring_buff, &enabler.at_pt),
                              at.error == false);

  ystring_cpy_fstring_to_buff(ATT_SAPBR_PASSWD);
  PT_SPAWN_AND_ASSERT_SUCCESS(pt, &enabler.at_pt,
                              at_set(ystring_buff, &enabler.at_pt),
                              at.error == false);

  ystring_cpy_fstring_to_buff(ATT_CMEE_2);
  PT_SPAWN_AND_ASSERT_SUCCESS(pt, &enabler.at_pt,
                              at_set(ystring_buff, &enabler.at_pt),
                              at.error == false);

  ystring_cpy_fstring_to_buff(ATT_SAPBR_SET);
  PT_SPAWN_AND_ASSERT_SUCCESS(pt, &enabler.at_pt,
                              at_set(ystring_buff, &enabler.at_pt),
                              at.error == false);

  /* we're done */
  enabler.error = false;

  PT_END(pt);
}

/* ****************************************************** */
/* Disabling state machine                                */
/* ****************************************************** */
static struct {
  struct pt pt;

  bool error;
} disabler;

static PT_THREAD(disabling_fiber(struct pt *pt)) {
  PT_BEGIN(pt);

  ystring_cpy_fstring_to_buff(ATT_CGATT_0);
  PT_SPAWN(pt, &at.pt, at_set(ystring_buff, &at.pt));
  PT_SPAWN(pt, &pwr_cycler.pt, power_cycle(&pwr_cycler.pt));

  modem.signal_strength = 0;

  PT_END(pt);
}

/* ****************************************************** */
/* Transmiting state machine                              */
/* ****************************************************** */
static struct {
  struct pt pt;    /* this fiber */
  struct pt at_pt; /* AT commands */
  uint8_t bytes_read;

  yerrno_t error;
} transmitter;

static PT_THREAD(transmitting_fiber(struct pt *pt)) {
  static int i;
  static char *line;
  static uint16_t len;
  static uint16_t read;
  static uint16_t http_response_len;
  static uint16_t http_read_len;

  PT_BEGIN(pt);

  transmitter.bytes_read = 0;
  transmitter.error = Y_ERROR;

  /* http init */
  ystring_cpy_fstring_to_buff(ATT_HTTPINIT);
  PT_SPAWN_AND_ASSERT_SUCCESS(pt, &transmitter.at_pt,
                              at_set(ystring_buff, &transmitter.at_pt),
                              at.error == false);

  ystring_cpy_fstring_to_buff(ATT_HTTPPARA_SET_CONTEXT_ID);
  PT_SPAWN_AND_ASSERT_SUCCESS(pt, &transmitter.at_pt,
                              at_set(ystring_buff, &transmitter.at_pt),
                              at.error == false);

  /* set URL */
  ystring_cpy_fstring_to_buff(ATT_HTTPARA_URL_PREFIX);
  ystring_append(ystring_buff, YSTRING_BUFF_LEN, modem.url);
  ystring_append(ystring_buff, YSTRING_BUFF_LEN, "\"");
  PT_SPAWN_AND_ASSERT_SUCCESS(pt, &transmitter.at_pt,
                              at_set(ystring_buff, &transmitter.at_pt),
                              at.error == false);

  /* set action - TODO GET ONLY NOW */
  ystring_cpy_fstring_to_buff(ATT_HTTP_ACTION_0);
  PT_SPAWN(pt, &transmitter.at_pt,
           serial_write(ystring_buff, &transmitter.at_pt));

  PT_SPAWN(pt, &transmitter.at_pt, serial_write(CRLF, &transmitter.at_pt));

  /*
  SIM900 is nasty as it can either reply with OK and then command result, or
  first the command result and then OK.

  Below I'm doing a trick - I expect two responses, and if it is OK then I
  ignore it
  */
  for (i = 0; i < 2; i++) {
    PT_SPAWN_AND_ASSERT_SUCCESS(
        pt, &at.wait_for_line_at,
        at_wait_for_line(&at.wait_for_line_at, HTTP_TIMEOT), at.error == false);
    line = get_line(NULL);
    ystring_cpy_fstring_to_buff(ATT_OK);
    if (ystring_equals(ystring_buff, line, ystring_strlen(line)) == false) {
      /* parse response */
      if (parse_at_response(line, ystring_strlen(line)) == Y_ERROR) {
        PT_EXIT(pt);
      }
    } else {
      /* ignore OK */
    }
    clear_serial_buffer();
  }

  /* check the http result code and response length */
  if (at.param_count != 3) {
    /* it means httpread did not get us result*/
    PT_EXIT(pt);
  } else if (at.params.param2 != 200) {
    /* did not get proper reply */
    ystring_cpy_fstring_to_buff(ATT_HTTP_TERM);
    PT_SPAWN_AND_ASSERT_SUCCESS(pt, &transmitter.at_pt,
                                at_set(ystring_buff, &transmitter.at_pt),
                                at.error == false);

    PT_EXIT(pt);
  }
  http_response_len = at.params.param3;
  /*
          if (http_response_len == 0) {
                  http_response_len = 1;
          }
  */
  read = 0;
  while (read < http_response_len) {
    /* send http read request */
    ystring_cpy_fstring_to_buff(ATT_HTTP_READ);
    PT_SPAWN(pt, &transmitter.at_pt,
             serial_write(ystring_buff, &transmitter.at_pt));

    PT_SPAWN(pt, &transmitter.at_pt, serial_write(CRLF, &transmitter.at_pt));

    /* read how many bytes we can read */

    /*
    PT_SPAWN_AND_ASSERT_SUCCESS(pt, &at.wait_for_line_at,
    at_wait_for_line(&at.wait_for_line_at), at.error == false); line =
    get_line(NULL);

    if (parse_at_response(line, ystring_strlen(line)) == Y_ERROR) {
            PT_EXIT(pt);
    } */

    for (i = 0; i < 2; i++) {
      ytrace(i);
      PT_SPAWN_AND_ASSERT_SUCCESS(
          pt, &at.wait_for_line_at,
          at_wait_for_line(&at.wait_for_line_at, HTTP_TIMEOT),
          at.error == false);
      line = get_line(NULL);
      ytracestr(line);
      ystring_cpy_fstring_to_buff(ATT_OK);
      if (ystring_equals(ystring_buff, line, ystring_strlen(line)) == false) {
        /* parse response */
        if (parse_at_response(line, ystring_strlen(line)) == Y_ERROR) {
          clear_serial_buffer();
          ystring_cpy_fstring_to_buff(ATT_HTTP_TERM);
          PT_SPAWN_AND_ASSERT_SUCCESS(pt, &transmitter.at_pt,
                                      at_set(ystring_buff, &transmitter.at_pt),
                                      at.error == false);

          PT_EXIT(pt);
        } else {
          i = 2; /* trick to exit the loop - I don't know how fibers will work
                    if break is used */
        }
      } else {
        /* ignore OK */
      }
      clear_serial_buffer();
    }

    http_read_len = at.params.param1;
    http_response_len = http_read_len; // TODO how AT works here?
    serial_consumer.mode = BYTE;

    /* read byte after byte */
    serial_consumer.mode = BYTE;

    while (http_read_len > 0) {
      /* setup timeout */
      at.timeout_expired = false;
      at.timeout_id = ytimer_setup_onetime_callback(at_cb, AT_ACK_TIMEOUT);

      PT_WAIT_UNTIL(pt, serial_consumer.buffer_full == true ||
                            at.timeout_expired == true);

      if (at.timeout_expired) {
        ytracestr("BUG");
        serial_consumer.mode = LINE;
        PT_EXIT(pt);
      }

      ytimer_reset_onetime_callback(at.timeout_id);

      if (read < modem.msg_buff_len) {
        modem.msg_buff[read] = serial_consumer.byte;
        transmitter.bytes_read++;
      }
      read++;
      clear_serial_buffer();
      http_read_len--;
    }

    /* wait for ok*/
    serial_consumer.mode = LINE;
    PT_SPAWN_AND_ASSERT_SUCCESS(pt, &at.ok_pt, at_wait_for_ok(&at.ok_pt),
                                at.error == false);
  }

  /* terminate http session */
  ystring_cpy_fstring_to_buff(ATT_HTTP_TERM);
  PT_SPAWN_AND_ASSERT_SUCCESS(pt, &transmitter.at_pt,
                              at_set(ystring_buff, &transmitter.at_pt),
                              at.error == false);

  transmitter.error = Y_OK;

  PT_END(pt);
}

/* ****************************************************** */
/* Main state machine                                     */
/* ****************************************************** */
extern void modem_inactivity_cb(void *cookie) {
  modem.inactivity_timer_expired = true;
}

static void notify_if_needed(yerrno_t err) {
  if (modem.result_callback != NULL) {
    modem.result_callback(err);
    modem.result_callback = NULL;
  }
}

static PT_THREAD(main_fiber(struct pt *pt)) {
  PT_BEGIN(pt);
  while (1) {
    /* disconnected state */
    do {
      set_serial_consumer_mode(LINE);
      clear_serial_buffer();
      PT_WAIT_UNTIL(pt, is_serial_ready() || modem.state == ENABLING);

      if (is_serial_ready()) {
        /* we're disabled and not interested */
        clear_serial_buffer();
      }
    } while (modem.state == DISABLED);

    /* enabling */
    PT_SPAWN(pt, &enabler.pt, enablng_fiber(&enabler.pt));

    if (enabler.error) {
      modem.state = DISABLED;
      modem.signal_strength = 0;
      modem.result_callback(Y_ERROR);
      modem.http_callback = NULL;

      if (enabler.power_off_on_error) {
        /* power off as it was powered on when setting up the modem */
        PT_SPAWN(pt, &pwr_cycler.pt, power_cycle(&pwr_cycler.pt));
        enabler.power_off_on_error = false;
      }

      break;
    } else {
      modem.state = ENABLED;
      notify_if_needed(Y_OK);
    }

    /* now modem is enabled - it can either start transmission or be disabled */
    do {
      modem.inactivity_timer_expired = false;
      modem.inactivity_timer_id = ytimer_setup_onetime_callback(
          modem_inactivity_cb, MODEM_INACTIVITY_TIMEOUT);

      /* check for messages in queue */
      if (modem.http_callback != NULL) {
        modem.state = READY_TO_TRANSMIT;
      }

      PT_WAIT_UNTIL(pt, modem.inactivity_timer_expired ||
                            modem.state == READY_TO_TRANSMIT ||
                            modem.state == DISABLING);

      if (modem.inactivity_timer_expired) {
        modem.state = DISABLING;
      } else if (modem.state == READY_TO_TRANSMIT) {

        ytimer_reset_onetime_callback(modem.inactivity_timer_id);

        PT_SPAWN(pt, &transmitter.pt, transmitting_fiber(&transmitter.pt));
        ytrace(200);
        modem.http_callback(transmitter.error, modem.msg_buff,
                            transmitter.bytes_read);
        ytrace(201);

        modem.http_callback = false;
        modem.msg_buff_len = 0;
        modem.msg_buff = 0;
        modem.url = 0;

        modem.state = ENABLED;
      }

      else if (modem.state == DISABLING) {
        ytimer_reset_onetime_callback(modem.inactivity_timer_id);
      } else {
        yassert(false); // cannot be here
      }

    } while (modem.state != DISABLING);

    /* now we are in disabling state*/
    PT_SPAWN(pt, &enabler.pt, disabling_fiber(&disabler.pt));
    modem.state = DISABLED;

    notify_if_needed(Y_OK);
  }
  PT_END(pt);
}

void ymodem_set_is_enabled(bool is_enabled, ymodem_callback_fn cb) {
  modem.error = Y_OK;

  if (is_enabled == true) {
    switch (modem.state) {
    case DISABLED:
      modem.result_callback = cb;
      modem.state = ENABLING;
      break;
    case ENABLED:
      modem.result_callback = cb;
      modem.state = DISABLING;
      break;
    default:
      cb(Y_BUSY);
    }
  } else {
    switch (modem.state) {
    case DISABLED:
      cb(Y_OK);
      break;
    case ENABLED:
      modem.result_callback = cb;
      modem.state = DISABLING;
      break;
    default:
      cb(Y_BUSY);
    }
  }
}

void ymodem_connect(yserial_t serial, const char *apn_name, const char *ip,
                    const char *user, const char *password) {
  yassert(YMODEM_EVENT_LOOP_SIZE < 256); /* some code here depends on it */

  modem.apn_name = apn_name;
  modem.user = user;
  modem.password = password;

  serial_consumer.mode = LINE;
  serial_consumer.serial = serial;

  modem.state = DISABLED;

  PT_INIT(&serial_consumer.pt);
  PT_INIT(&modem.pt);
}

bool ymodem_is_busy() {
  return !(modem.state == DISABLED || modem.state == ENABLED ||
           modem.state == DISABLING);
}

/**
\brief used to get resource from the internet; this function will take care
of the power management, but there may be only one active request (regardless of
the type) at any given moment.

\param url URL of the web page to get
\param buff buffer where the output will be stored
\param buff_len length of th buffer
*/

void http_enabler_cb_fn(yerrno_t error) {
  if (error != Y_OK) {
    modem.http_callback(error, modem.msg_buff, 0);
    return;
  }

  modem.state = READY_TO_TRANSMIT;
}

void ymodem_http_get(const char *url, ymodem_http_get_callback_fn cb,
                     uint8_t *buff, uint8_t buff_len) {
  bool is_powered_on;
  int32_t ignored;

  // check input
  yassert(url != NULL);
  yassert(buff != NULL);
  yassert(cb != NULL);
  yassert(buff_len > 0);

  if (ymodem_is_busy()) {
    cb(Y_BUSY, buff, 0);
  }

  modem.url = url;
  modem.msg_buff = buff;
  modem.msg_buff_len = buff_len;
  modem.http_callback = cb;

  ymodem_get_status(&is_powered_on, (int8_t *)&ignored);
  if (!is_powered_on) {
    ymodem_set_is_enabled(true, http_enabler_cb_fn);
  } else {
    yassert(modem.state == ENABLED || modem.state == ENABLING);

    if (modem.state == ENABLED) {
      modem.state = READY_TO_TRANSMIT;
    }
  }
}

/**
\brief Gets the modem status

This is very lazy function - it gets the status from the data set by
other functions, but do not actively ask modem for it.

\param modem modem to measure
\param is_powered_on pointer to the user allocated variable telling if the modem
is currently powered on \param signal_strength pointer to the user allocated
variable holding signal strength in 0 (no signal) to 127 (strongest)
*/
void ymodem_get_status(bool *is_powered_on, int8_t *signal_strength) {
  *is_powered_on = (modem.state == DISABLED || modem.state == DISABLING ||
                    modem.state == ENABLING)
                       ? 0
                       : 1;
  *signal_strength = modem.signal_strength;
}

void ymodem_disconnect(void) { serial_consumer.serial = NULL; }

void ymodem_tick(void) {
  if (serial_consumer.serial != NULL) {
    PT_SCHEDULE(serial_consumer_fiber(&serial_consumer.pt));
    PT_SCHEDULE(main_fiber(&modem.pt));
  }
}

// setup
//

// NORMAL POWER DOWN --> poszlo napiecie
// ERROR to error
// OK to OK
//
// 0. AT --> OK albo nic
//
// 1. Ustawienie verbose na bledy
// AT+CEER=0 --> OK
//
// 2. wylaczenie echa + sprawdzenie, czy jest ok
// ATE0 --> OK
//
// 3. Sprawdzenie sily sygnalu i operatora
//
// AT+COPS? --> +COPS: 0,0,"PLAY" \n  OK \n nazwa operatora --> po tym mozna
// autokonfa zrobic AT+CSQ --> +CSQ: 12,0 \n OK \n
//
// 4. Sprawdzenie, czy jest w sieci
// AT+CREG? --> +CREG: 0,1 albo +CREG: 0,5
//
// 5. sprawdzenie, czy juz jestesmy podlaczeni do grps
// AT+CGATT? --> +CGATT: 1 --> 1 = AT+CGATT=0 --> OK
// # to mozna sobie darowac... AT+CGCLASS? -->  +CGCLASS: "B" --> inaczej error
// 6. Ustawienie kontekstu PDP
//   AT+CGDCONT=1,"IP","internet GPRS", "0.0.0.0" --> OK
//
// 7. setting the SAPBR, the connection type is using gprs
// AT+SAPBR=3,1,"CONTYPE","GPRS" --> OK
//
// 8. setting the APN, the second need you fill in your local apn server
// AT+SAPBR=3,1,"APN","Internet" --> OK
//
// 9. setting the APN, the second need you fill in your username (nie wywoluj
// jak nie ma usera) AT+SAPBR=3,1,"USER","Internet" --> OK
//
// 10. Setting the password (nie wywoluj jak nie ma hasla)
// AT+SAPBR=3,1,"PWD","Internet" --> OK

// AT+CGATT? --> +CGATT: 1 --> 0 = AT+CGATT=1 --> OK
//
// 11. setting the SAPBR
// AT+SAPBR=1,1 --> OK
//
// 12. http init
// AT+HTTPINIT --> OK
//
// 13. http paramerers
// AT+HTTPPARA="URL","www.google.com" -> OK
//
// 14. wywolanie
// AT+HTTPACTION=0 --> OK, a potem +HTTPACTION:0, 603, 0 --> 600 to lipa
// (kolejnosc moze byc odwrotna)
//							+HTTPACTION:0,302,258 -> 302 to
//ok, do odczytani 258 bajtow  --> 302 do redirect
//
// 15. odczyt AT+HTTPREAD --> OK

// const char *setup_string = "AT+CERR=0;+CGDCONT=1,\"IP\",\"internet
// GPRS\",\"0.0.0.0\";+CSQ;+SAPBR=3,1,\"CONTYPE\",\"GPRS\";+SAPBR=3,1,\"APN\",\"Internet\";+SAPBR=3,1,\"USER\",\"Internet\";+SAPBR=3,1,\"PWD\",\"Internet\";+SAPBR=1,1";
