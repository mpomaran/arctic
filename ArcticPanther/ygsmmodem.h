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

#ifndef YGSM_MODEM_H_HAS_BEEN_INCLUDED
#define YGSM_MODEM_H_HAS_BEEN_INCLUDED

#include "yserial.h"
#include <stdint.h>

/********************************************************************************

DISCLAIMER

The main purpose if this library is to send HTTP requests, thus most important
functions are http requests.

Code is under development, and I didn't pay much attention to visibility of
other funtions, but as this matures you may expect some of power management
functions will be private; user is not advised to call it directly.

*********************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**
\brief Defines how long in ms modem is allowed to spend in idle state before
being powered off
*/
#define MODEM_INACTIVITY_TIMEOUT (30000)

typedef void (*ymodem_callback_fn)(yerrno_t);

/**
\brief Callback used after content from the net is requested; paramerers are:
   - error no
   - pointer to the buffer with the content (if successful)
   - how many bytes were read
*/
typedef void (*ymodem_http_get_callback_fn)(yerrno_t, uint8_t *, uint8_t);

/**
\brief used to get resource from the internet; this function will take care
of the power management, but there may be only one active request (regardless of
the type) at any given moment.

\param url URL of the web page to get; must not be modified until call has
completed \param buff buffer where the output will be stored \param buff_len
length of th buffer
*/
void ymodem_http_get(const char *url, ymodem_http_get_callback_fn callback,
                     uint8_t *buff, uint8_t buff_len);

/**
\brief Connects to the modem using provided pins and returns new instance

\param connection is an open connection to the modem
\param apnName name of the APM given by the provider (needs to be available for
the whole life of the ymodem_t object) \param ip IP or NULL if IP is to be
dynamically provided (needs to be available for the whole life of the ymodem_t
object) \param user internet user name (needs to be available for the whole life
of the ymodem_t object), null if not used \param password for the internet user
(needs to be available for the whole life of the ymodem_t object), null if not
used

*/
void ymodem_connect(yserial_t connection, const char *apn_name, const char *ip,
                    const char *user, const char *password);

/**
\brief wakes up the modem, turning it on if neceasry

\param callback function to be called when operation is completed (either with
success or error)
*/
void ymodem_set_is_enabled(bool is_enabled, ymodem_callback_fn cb);

/**
\brief Gets the modem status

\param is_powered_on pointer to the user allocated variable telling if the modem
is currently powered on \param signal_strength pointer to the user allocated
variable holding signal strength in 0 (no signal) to 127 (strongest)
*/
void ymodem_get_status(bool *is_powered_on, int8_t *signal_strength);

bool ymodem_is_busy(void);
void ymodem_disconnect(void);

void ymodem_tick(void);

#ifdef __cplusplus
}
#endif

#endif
