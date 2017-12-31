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

#ifndef _WIN32
#error Wrong config!!!
#endif

#ifndef YCONFIG_H_HAS_BEEN_INCLUDED
#define YCONFIG_H_HAS_BEEN_INCLUDED

#include <memory.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

typedef enum { Y_LOW = 0, Y_HIGH } pin_state_t;

#define PROGMEM
#define read_uint32(ptr) (*(uint32_t *)(ptr))
#define read_uint8(ptr) (*(uint8_t *)(ptr))
#define read_ptr(ptr) (*(uint8_t **)ptr)

#define memcpy_P memcpy

#ifdef __cplusplus
}
#endif

#endif
