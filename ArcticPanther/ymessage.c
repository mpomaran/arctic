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

#include "ymessage.h"

#include <yassert.h>
#include <yrand.h>
#include <ystring.h>
#include <ytimer.h>

#include "secrets.h"

uint8_t nibble_to_char(uint8_t nibble) {
  if (nibble < 10)
    return '0' + nibble;
  else
    return 'A' + nibble - 10;
}

/*
output always 17 = 16 bytes + 1 (zero) byte
*/
uint8_t uint64_to_bytes(char *buff, uint64_t number) {
  uint8_t i;
  for (i = 0; i < sizeof(uint64_t); i++) {
    uint8_t b = 0xff & (number >> ((sizeof(uint64_t) - i - 1) * 8));

    buff[i * 2 + 1] = nibble_to_char(b & 0xf);
    buff[i * 2] = nibble_to_char((b >> 4) & 0xf);
  }
  buff[sizeof(uint64_t) * 2] = 0;

  return sizeof(uint64_t) * 2 + 1;
}

yerrno_t ymessage_construct(uint8_t *buff, uint8_t buff_len, uint64_t payload,
                            ymessage_cb message_ready_cb) {
  uint64_t timestamp = ytimer_get_uptime_in_ms();
  uint64_t random = yrand_rand();
  uint8_t i;
  uint64_t encoded_payload = 0;

  yassert(buff_len >= (sizeof(uint64_t) * 3 * 2 + 1));

  buff += uint64_to_bytes(buff, timestamp) - 1;
  buff += uint64_to_bytes(buff, random) - 1;

  yrand_seed(timestamp ^ random, SECRET_KEY);

  for (i = 0; i < sizeof(payload); i++) {
    uint8_t b = payload >> (i * 8) & 0xff;
    uint8_t key = (uint8_t)(yrand_rand() & 0xff);
    encoded_payload += ((uint64_t)(b ^ key)) << (i * 8);
  }

  buff += uint64_to_bytes(buff, encoded_payload);

  message_ready_cb();

  return Y_OK;
}

void ymessage_tick() {
  // TODO
}
