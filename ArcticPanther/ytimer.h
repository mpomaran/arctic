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

#ifndef YTIMER_H_INCLUDED
#define YTIMER_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <ygsmmodem.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ytimer_cb)();

typedef uint8_t timer_id_t;

/**
\brief Sets up a timeout which is going to be fired in a specific time
*/
timer_id_t ytimer_setup_onetime_callback(ytimer_cb callback,
                                         uint32_t delay_in_ms);

/**
\brief reses previously created timeout
*/
void ytimer_reset_onetime_callback(timer_id_t idx);

/**
\brief gets the uptime in milliseconds
*/
uint64_t ytimer_get_uptime_in_ms();

/**
\brief Provides time measurement and executes the main timer loop

\param current_ms current time in millis; if less than previously provided it
means the uint32_t rolled over
*/
void ytimer_tick(uint32_t current_ms);

/**
\brief Resets the internal timer

\param current_uptime_ms new time to be set
*/
void ytimer_set_uptime_in_ms(uint64_t current_uptime_ms);

void ytimer_delay(uint32_t delay);

/* use to clean up after tests */
void ytimer_force_init();

uint32_t ytimer_millis();
void ytimer_millis_add_skew(int32_t skew);

#ifdef __cplusplus
}
#endif

#endif /* of YTIMER_H_INCLUDED */
