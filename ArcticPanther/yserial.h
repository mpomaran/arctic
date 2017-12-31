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

#ifndef Y_SERIAL_H_HAS_BEEN_INCLUDED
#define Y_SERIAL_H_HAS_BEEN_INCLUDED

#include "ytypes.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <ystring.h>

#ifdef __cplusplus
extern "C" {
#endif

struct yserial;
typedef struct yserial *yserial_t;

typedef void (*yserial_test_data_receiver_callback_fn)(yserial_t serial,
                                                       void *cookie,
                                                       const uint8_t *buff,
                                                       size_t buff_len);

yserial_t yserial_alloc(uint32_t baud_speed, uint8_t rx_pin, uint8_t tx_pin);

yerrno_t yserial_free(yserial_t serial);

/**
\brief returns true if there is something pending in the buffer
*/
bool yserial_available(yserial_t serial);

/**
\brief reads one byte from serial; behavior is undefined if no bytes are
available
*/
uint8_t yserial_read_byte(yserial_t serial);

/**
\brief reads amount of bytes from serial; behavior is undefined if no bytes are
pending
*/
size_t yserial_read_bytes(yserial_t serial, uint8_t *buff, size_t buff_len);

/**
\brief writes specified amount of data; blocks until all bytes are written

\param serial connection to be used
\param buff which holds the data to be send
\param buff_len indicates amount of bytes to be send

\return amount of bytes wrote
*/
size_t yserial_write_data(yserial_t serial, const uint8_t *buff,
                          size_t buff_len);

/**
\brief writes specified amount of data; blocks until all bytes are written

\param serial connection to be used
\param str which holds the data to be send - trailing 0 will NOT be sent

\return amount of bytes wrote
*/
size_t yserial_write_string(yserial_t serial, const char *str);

/**
\brief writes specified amount of data; blocks until all bytes are written

\param serial connection to be used
\param str which holds the data to be send - trailing 0 will NOT be sent
*/
void yserial_write_fstring(yserial_t serial, fstring_id str);

/**
\brief TEST API - when called then adds to the receiving queue specifie data

\return number of bytes sent to the yserial client
*/
size_t yserial_test_simulate_received(yserial_t serial, const uint8_t *buff,
                                      size_t buff_len);

/**
\bref TEST API - sets callback which is to be called each time user uses "write"
AND enables test mode

\param serial connection used
\param cb callback function or null if test mode should be disabled
*/
void yserial_test_set_fake_mode(yserial_t serial,
                                yserial_test_data_receiver_callback_fn cb,
                                void *cookie);

#ifdef __cplusplus
}
#endif

#endif /* of Y_SERIAL_H_HAS_BEEN_INCLUDED */
