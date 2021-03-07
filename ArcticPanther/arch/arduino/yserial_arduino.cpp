#include <Arduino.h>

//#define ALT_SERIAL

#ifdef ALT_SERIAL
#include <AltSoftSerial_Boards.h>
#include <AltSoftSerial.h>
#else
#include <SoftwareSerial.h>
#endif
#include <stdint.h>
#include <memory.h>
#include <stdlib.h>

#include "config.h"
#include <yserial.h>
#include <yassert.h>
#include <ycircbuf.h>
#include <ystring.h>


#ifndef ARDUINO_ARCH_AVR
#error This file is for Arduino only!!!
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define CIRCBUF_SIZE 32

	struct yserial
	{
#ifdef ALT_SERIAL
		AltSoftSerial *serial;
#else
		SoftwareSerial *serial;
#endif
		yserial_test_data_receiver_callback_fn callback; // used also as a flag if sim mode should be on
		ycircbuf_t buf; // used for simulation only
		void *cookie;
	};

  static bool	is_sim_mode (yserial_t serial) {
		return serial->callback != 0;
	}

	yserial_t	yserial_alloc(uint32_t baud_speed, uint8_t rx_pin, uint8_t tx_pin)
	{
		yserial_t result = (struct yserial *)new yserial(); //(sizeof(struct yserial));
#ifdef ALT_SERIAL
		yassert (rx_pin == 8 && tx_pin == 9);
		result->serial = new AltSoftSerial(rx_pin, tx_pin);
#else
		result->serial = new SoftwareSerial(rx_pin, tx_pin);
#endif
		result->serial->begin(baud_speed);
		result->callback = 0;
		return result;
	}

	yerrno_t yserial_free(yserial_t serial)
	{
		delete serial->serial;
		delete serial;
		return Y_OK;
	}

	bool		yserial_available(yserial_t serial)
	{
		if (is_sim_mode(serial)) {
			return ycircbuf_size(serial->buf) != 0;
		} else {
			return serial->serial->available();
		}
	}

	uint8_t		yserial_read_byte(yserial_t serial)
	{
		if (is_sim_mode(serial)) {
			yassert(false);
			uint8_t result;
			uint16_t bytes_read = ycircbuf_pop(serial->buf, &result, 1);
			yassert(bytes_read == 1);
			return result;
		}

		uint8_t result = serial->serial->read();

//		Serial.print("Rx:");
		Serial.print((char)result);
		Serial.flush();
		return result;
	}

	size_t		yserial_read_bytes(yserial_t serial, uint8_t *buff, size_t buff_len)
	{
		unsigned i;
		for (i = 0; i < buff_len; i++) {
			buff[i] = yserial_read_byte(serial);
		}
		return i;
	}

	size_t		yserial_write_data(yserial_t serial, const uint8_t *buff, size_t buff_len)
	{
		if (serial->callback != 0) {
			 serial->callback(serial, serial->cookie, buff, buff_len);
			 return buff_len;
		} else {
//			Serial.print("W:");
			for (int i = 0; i < buff_len; i++) {
				Serial.print((char)buff[i]);
			}
			Serial.flush();
//			Serial.println("");
			return serial->serial->write(buff, buff_len);
		}
	}

	size_t		yserial_write_string(yserial_t serial, const char *str)
	{
		size_t result = yserial_write_data(serial, (const uint8_t *)str, ystring_strlen(str));
		return result;
	}
	/**
	\brief TEST API - when called then adds to the receiving queue specifie data
	*/

	size_t		yserial_test_simulate_received(yserial_t serial, const uint8_t *buff, size_t buff_len)
	{
		yassert (is_sim_mode(serial) == true);
		return ycircbuf_append(serial->buf, buff, buff_len);
	}

	/**
	\bref TEST API - sets callback which is to be called each time user uses "write"
	*/
	void		yserial_test_set_fake_mode(yserial_t serial, yserial_test_data_receiver_callback_fn cb, const void* cookie)
	{
		if (is_sim_mode(serial)) {
			ycircbuf_free(serial->buf);
		}

		if (cb != 0) {
			serial->buf = ycircbuf_alloc (CIRCBUF_SIZE);
		}

		serial->callback = cb;
		serial->cookie = const_cast<void *>(cookie);
	}

#ifdef __cplusplus
}
#endif
