#include <config.h>
#include <Arduino.h>
#include "ysim900_emulator.h"
#include <ytimer.h>
#include <tests.h>

#ifndef NDEBUG
static ysim900_emu_t sim900;
#endif

void yhal_set_sim900_emu(void * emu)
{
#ifndef NDEBUG
	sim900 = (ysim900_emu_t)emu;
#endif
}

void yhal_set_pin_mode(uint8_t pin, pin_mode_t mode)
{
	pinMode(pin, mode == Y_PIN_MODE_INPUT ? INPUT : OUTPUT);
}

void yhal_set_pin_state(uint8_t pin, pin_state_t state)
{
	#ifdef RUN_YGSMMODEM_UNIT_TESTS
	if (pin == SIM900_POWER_PIN && sim900 != NULL) {
		ysim900_emu_set_power_control_pin(sim900, state);
	}
	else {
		digitalWrite(pin, state == Y_HIGH ? HIGH : LOW);
	}
	#else
		digitalWrite(pin, state == Y_HIGH ? HIGH : LOW);
	#endif
}

uint16_t yhal_read_analog(uint8_t pin)
{
#ifdef RUN_YGSMMODEM_UNIT_TESTS
	if (pin == SIM900_POWER_PIN && sim900 != NULL) {
		// TODO
		return pin;
	}
	else {
		return analogRead(pin);
	}
#else
		return analogRead(pin);
#endif
}

uint8_t yhal_read_digital(uint8_t pin)
{
#ifdef RUN_YGSMMODEM_UNIT_TESTS
	if (pin == SIM900_POWER_PIN && sim900 != NULL) {
		// TODO
		return pin;
	}
	else {
		return digitalRead(pin);
	}
#else
		return digitalRead(pin);
#endif
}

void ytimer_delay(uint32_t d) // cross-platform sleep function
{
#ifdef _WIN32
	Sleep(d);
#elif _POSIX_C_SOURCE >= 199309L
	struct timespec ts;
	ts.tv_sec = d / 1000;
	ts.tv_nsec = (d % 1000) * 1000000;
	nanosleep(&ts, NULL);
#else
	delay(d);
#endif
}

uint32_t yhal_millis()
{
	return millis();
}

/*
Improving Accuracy
While the large tolerance of the internal 1.1 volt reference greatly limits the accuracy of this measurement, for individual projects we can compensate for greater accuracy. To do so, simply measure your Vcc with a voltmeter and with our readVcc() function. Then, replace the constant 1125300L with a new constant:

scale_constant = internal1.1Ref * 1023 * 1000

where

internal1.1Ref = 1.1 * Vcc1 (per voltmeter) / Vcc2 (per readVcc() function)

This calibrated value will be good for the AVR chip measured only, and may be subject to temperature variation. Feel free to experiment with your own measurements.
*/
uint16_t yhal_read_vcc()
{
 // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  ytimer_delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint32_t result = ADCL;
  result |= ADCH<<8;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return (uint16_t)result; // Vcc in millivolts
}


