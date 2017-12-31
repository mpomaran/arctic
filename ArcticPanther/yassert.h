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

#ifndef YASSERT_HAS_BEEN_INCLUDED
#define YASSERT_HAS_BEEN_INCLUDED

#ifdef _WIN32
#include <assert.h> // for assert
#include <stdio.h>  // for printf
#include <stdlib.h> // for exit
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#ifndef NDEBUG
#define yassert assert
#define ytrace(param)                                                          \
  do {                                                                         \
    fprintf(stderr, "%s:%d - %d\n", __FILE__, __LINE__, (int)param);           \
  } while (0)
#define ytracestr(param)                                                       \
  do {                                                                         \
    fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, (char *)param);        \
  } while (0)
#else
#define yassert assert
#define ytrace(param)                                                          \
  do {                                                                         \
    fprintf(stderr, "%s:%d - %d\n", __FILE__, __LINE__, (int)param);           \
  } while (0)
#define ytracestr(param)                                                       \
  do {                                                                         \
    fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, (char *)param);        \
  } while (0)
#endif
#elif defined(ARDUINO_ARCH_AVR)

void __yassert(const char *__file, int __lineno);

#ifndef NDEBUG
void __ytrace(const char *__file, int __lineno, int32_t param);
void __ytracestr(const char *__file, int __lineno, const char *param);
#define yassert(cond)                                                          \
  do {                                                                         \
    if (!(cond)) {                                                             \
      __yassert("", __LINE__);                                                 \
    }                                                                          \
  } while (0)
#define ytrace(param)                                                          \
  do {                                                                         \
    __ytrace("", __LINE__, param);                                             \
  } while (0) // TODO
#define ytracestr(param)                                                       \
  do {                                                                         \
    __ytracestr("", __LINE__, param);                                          \
  } while (0)
#else
#define yassert(cond)                                                          \
  do {                                                                         \
    if (!(cond)) {                                                             \
      __yassert("", __LINE__);                                                 \
    }                                                                          \
  } while (0)
#define ytrace(param)                                                          \
  do {                                                                         \
    ;                                                                          \
  } while (0)
#define ytracestr(param)                                                       \
  do {                                                                         \
    ;                                                                          \
  } while (0)
#endif
#else
#error Unknown platform
#endif

#ifdef __cplusplus
}
#endif

#endif
