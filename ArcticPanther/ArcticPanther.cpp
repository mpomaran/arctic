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

#ifndef ARDUINO_ARCH_AVR

int main()
{
  y_unit_test();
  return 0;
}

#elif ARDUINO_ARCH_AVR

#ifndef __cplusplus
#error Need to link with C++
#endif

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "ygsmmodem.h"
#include "ytimer.h"
#include <pt.h>
#include <yassert.h>
#include <ymessage.h>
#include <yrand.h>
#include <yscript.h>
#include <yserial.h>
#include <ystring.h>

#include "secrets.h"

const static uint8_t BLINK_PIN = 13;
const static uint8_t NOISY_PIN = 12;
static uint8_t SENSOR_PIN = 1;
static uint16_t SENSOR_TYPE_PIN = 0;
static uint8_t POWER_PIN = 5;
static uint8_t GSM_POWER_PIN = 6;

const static uint32_t LOW_POWER_SLEEP_DURATION_MIN = 60;
const static uint32_t MILLIS_PER_MINUTE = 1000L * 60L; // TODO - change for real val.

static char *M2M_URL =
    (char *)MY_BRIDGE_PREFIX
    "cccccccccccccccc00112233445566778899aabbccddeeff";
static const uint8_t CONST_URL_PART_LEN = sizeof(MY_BRIDGE_PREFIX) - 1; /* constant part, not to be overwritten */
static const uint8_t PAYLOAD_LEN = 48;

void blink_setup(uint8_t pin = BLINK_PIN)
{
  yhal_set_pin_mode(pin, Y_PIN_MODE_OUTPUT);
}

static void enable_gsm_modem_power()
{
  ytrace(100);
  yhal_set_pin_mode(GSM_POWER_PIN, Y_PIN_MODE_OUTPUT);
  yhal_set_pin_state(GSM_POWER_PIN, Y_HIGH);
}

static void disable_gsm_modem_power()
{
  ytrace(101);
  yhal_set_pin_mode(GSM_POWER_PIN, Y_PIN_MODE_OUTPUT);
  yhal_set_pin_state(GSM_POWER_PIN, Y_LOW);
}

void blink(unsigned long duration, uint8_t pin = BLINK_PIN)
{
  yhal_set_pin_state(pin, Y_HIGH);
  ytimer_delay(duration);         // wait for a second
  yhal_set_pin_state(pin, Y_LOW); // set the LED off
  ytimer_delay(duration);         // wait for a second
}

yserial_t serial;

static enum {
  WAITING_FOR_POWER,
  POWER_ON,
  REQUEST_SENT,
  TURNING_OFF_MODEM,
  WAIT_FOR_MODEM_OFF,
  LOW_POWER_SLEEP
} comm_state;

static void on_enter_waiting_for_power();
static void on_enter_low_power_sleep();
static void build_some_entropy();

void power_callback(yerrno_t result);
void power_off_callback(yerrno_t t);

void resume_power_callback();

const static int NETWORK_DATA_BUFF_LEN = 128;
static uint8_t network_data_buff[NETWORK_DATA_BUFF_LEN];
static uint32_t minutes_spent_in_low_power_state;
static bool encryption_completed;

static void on_enter_low_power_sleep()
{
  ytrace(ytimer_millis());

  ytimer_setup_onetime_callback(resume_power_callback, MILLIS_PER_MINUTE);
  minutes_spent_in_low_power_state = 0;
  comm_state = TURNING_OFF_MODEM;
}

void message_created_callback() { encryption_completed = true; }

static uint32_t read_sensor()
{
  uint32_t result;
  uint16_t sensor_reading;
  double sensor_reading_temp = 0; // double to slow down the CPU and maitain needed accuracy
  uint16_t sensor_type_reading;
  uint16_t vcc;
  uint8_t sample;
  uint32_t samples;
  const uint8_t MAX_SAMPLES = 128;

  yhal_set_pin_mode(SENSOR_PIN, Y_PIN_MODE_INPUT);
  yhal_set_pin_mode(POWER_PIN, Y_PIN_MODE_OUTPUT);
  yhal_set_pin_state(POWER_PIN, Y_HIGH);
  ytimer_delay(1000); // wait for hardware to stabilize somehow

  for (int lp = 0; lp < 50; lp++)
  {
    yhal_read_analog(SENSOR_PIN); // Throw away 1st readings to allow ADC to settle
  }

  sensor_reading_temp = 0;
  for (int i = 0; i < MAX_SAMPLES; i++)
  {
    sensor_reading_temp += yhal_read_analog(SENSOR_PIN) & 1023;
  }
  ;
  sensor_reading = (uint16_t)(sensor_reading_temp / MAX_SAMPLES);

  sensor_type_reading = (yhal_read_analog(SENSOR_TYPE_PIN) >> 1) & 511;
  vcc = yhal_read_vcc();
  ytrace(vcc);

  if (vcc < 4000)
  {
    vcc = 0;
  }
  else if (vcc > 6047)
  {
    vcc = 1023;
  }
  else
  {
    vcc -= 4000;
    vcc /= 2;
  }
  vcc &= 1023;

  yhal_set_pin_state(POWER_PIN, Y_LOW);
  ytrace(vcc);
  ytrace(sensor_reading);
  ytrace(sensor_type_reading);

  return (uint32_t)sensor_reading + ((uint32_t)vcc << 11) +
         ((uint32_t)sensor_type_reading << 22);
}

static void on_enter_waiting_for_power()
{
  comm_state = WAITING_FOR_POWER;

  uint32_t sensor_value = read_sensor();

  uint64_t payload = (uint64_t)sensor_value + ((uint64_t)sensor_value << 32);
  encryption_completed = false;

  ytracestr(M2M_URL);

  ymessage_construct((uint8_t *)(M2M_URL + CONST_URL_PART_LEN), PAYLOAD_LEN + 1,
                     payload, message_created_callback);

  ytracestr(M2M_URL);

  enable_gsm_modem_power();
  ymodem_set_is_enabled(true, power_callback);
  ytrace(ytimer_millis());
  ytrace(1);
}

// TODO rename
void resume_power_callback()
{
  build_some_entropy();
  minutes_spent_in_low_power_state++;
  ytrace(LOW_POWER_SLEEP_DURATION_MIN - minutes_spent_in_low_power_state);
  if (minutes_spent_in_low_power_state >= LOW_POWER_SLEEP_DURATION_MIN)
  {
    on_enter_waiting_for_power();
    //		ytrace(ytimer_millis());
    //		ytrace(1);
  }
  else
  {
    ytimer_setup_onetime_callback(resume_power_callback, MILLIS_PER_MINUTE);
    //		ytrace(ytimer_millis());
    //		ytrace(1);
  }
}

// TODO rename
void power_callback(yerrno_t result)
{
  yassert(comm_state == WAITING_FOR_POWER);
  ytrace(ytimer_millis());

  if (result != Y_OK)
  {
    ytracestr("P0");
    on_enter_waiting_for_power();
  }
  else
  {
    ytracestr("P1");
    comm_state = POWER_ON;
  }
}

void power_off_callback(yerrno_t result)
{
  ytrace(result);
  if (result != Y_OK)
  {
    on_enter_low_power_sleep();
  }
  else
  {
    comm_state = LOW_POWER_SLEEP;
  }
}

void http_callback(yerrno_t result, uint8_t *buff, uint8_t len)
{
  ytrace(50);
  yassert(comm_state == REQUEST_SENT);
  yassert(len <= NETWORK_DATA_BUFF_LEN);

  if (result == Y_OK)
  {
    ytracestr("H+");
    for (int i = 0; i < len; i++)
    {
      Serial.write(buff[i]);
    }
  }
  else
  {
    ytracestr("H-");
  }
  on_enter_low_power_sleep();
}

uint8_t read_bit_from_noisy_pin() { yhal_read_digital(NOISY_PIN); }

uint64_t read_noisy_64_bits()
{
  uint64_t result = 0;
  for (uint8_t i = 0; i < 64; i++)
  {
    result |= (uint64_t)read_bit_from_noisy_pin() << i;

    ytimer_delay(10);
  }
  return result;
}

void seed_randomness()
{
  yhal_set_pin_mode(NOISY_PIN, Y_PIN_MODE_INPUT);
  yrand_seed(read_noisy_64_bits() + 17, read_noisy_64_bits() + 11);
}


#if 0
void run_test_in_endless_loop()
{
  yhal_set_pin_mode(SENSOR_PIN, Y_PIN_MODE_INPUT);
  yhal_set_pin_mode(POWER_PIN, Y_PIN_MODE_OUTPUT);
  yhal_set_pin_state(POWER_PIN, Y_HIGH);
  ytimer_delay(1000); // wait for hardware to stabilize


  while(1) {
    double vcc = yhal_read_vcc(); /* in volts */

    for (int lp = 0; lp < 50; lp++)
    {
      yhal_read_analog(SENSOR_PIN); // Throw away 1st readings to allow ADC to settle
    }

    double sensor_reading = 0;
    for (int i = 0; i < 128; i++) {
      sensor_reading += yhal_read_analog(SENSOR_PIN) & 1023;
    }
    sensor_reading /= 128;

    ytrace((int)(sensor_reading));

    // convert reading to millivolts
    sensor_reading /= 1024;
    sensor_reading *= vcc;

    // measure delta between reading and 750 mv
    // using 10mV/C formula caltulate delta in C between this value and 25C
    double deltaC = (750 - sensor_reading) / 10;
    double temp = 25 - deltaC;

    ytrace((int)(temp * 100.0));

    blink(500);
  }
}

#endif


void setup()
{
  blink_setup(); // Setup for blinking
  for (int i = 0; i < 10; i++)
  {
    blink(100);
  }

#if 1
  yconfig_initialize_if_needed();

  for (int i = 0; i < 3; i++)
  {
    blink(500);
  }
  yassert(ystring_strlen(M2M_URL) == CONST_URL_PART_LEN + PAYLOAD_LEN);

  //	Serial.begin(4800);
  blink_setup(); // Setup for blinking
  for (int i = 0; i < 10; i++)
  {
    blink(100);
  }

  y_unit_test();

  ytimer_set_uptime_in_ms(0);
  serial = yserial_alloc(19200, 7, 8);
  ymodem_connect(serial, NULL, NULL, NULL, NULL);

  seed_randomness();

  on_enter_waiting_for_power();
#endif
}

void build_some_entropy()
{
  yrand_rand(); // generate another random number

  if (read_bit_from_noisy_pin())
  {
    yrand_rand(); // generate another random number
  }
}

void loop()
{
  static uint64_t prev_blink = 0;
#if 0
	yhal_enter_ms_sleep();
	enable_gsm_modem_power();
#endif
#if 1
  ytimer_tick(ytimer_millis());
  ymodem_tick();

  build_some_entropy();
  if (ytimer_get_uptime_in_ms() - prev_blink > 1000)
  {
    blink(10);
    prev_blink = ytimer_get_uptime_in_ms();
  }

  // TODO: feed data to the script
  yscript_tick();
  // TODO: action based on script

  switch (comm_state)
  {
  case WAITING_FOR_POWER:
  {
    build_some_entropy();
    if (encryption_completed)
    {
      // NOP
    }
    else
    {
      ymessage_tick();
    }
    break;
  }
  case POWER_ON:
  {
    if (encryption_completed)
    {
      bool is_powered_on = 42;
      int8_t signal_strength = 42;

      ymodem_get_status(&is_powered_on, &signal_strength);
      yassert(is_powered_on == true);

      // send request
      ytracestr(M2M_URL);

      ymodem_http_get(M2M_URL, http_callback, network_data_buff,
                      NETWORK_DATA_BUFF_LEN);

      comm_state = REQUEST_SENT;
      encryption_completed = false;
    }
    else
    {
      ymessage_tick();
    }

    break;
  }
  case REQUEST_SENT:
  {
    build_some_entropy();
    break;
  }
  case TURNING_OFF_MODEM:
  {
    ymodem_set_is_enabled(false, power_off_callback);
    comm_state = WAIT_FOR_MODEM_OFF;
    break;
  }
  case WAIT_FOR_MODEM_OFF:
  {
    break;
  }
  case LOW_POWER_SLEEP:
  {
    disable_gsm_modem_power();
    ytrace(ytimer_millis());
    yhal_enter_ms_sleep();
    break;
  }
  default:
    yassert(false);
  }
#endif
}

#else

#error Unknown platform

#endif
