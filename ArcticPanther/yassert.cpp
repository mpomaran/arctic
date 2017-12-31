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

#include "yassert.h"

#ifdef ARDUINO_ARCH_AVR

#include <Arduino.h>
#include <stdint.h>

extern char *__brkval;         /* first location not yet allocated */
extern size_t __malloc_margin; /* user-changeable before the first malloc() */
extern char *__malloc_heap_start;
extern char *__malloc_heap_end;

static bool speedSet = false;

static void _lazySerialInit() {
  if (!speedSet) {
    Serial.begin(4800);
    speedSet = true;
  }
}

static void separator() { Serial.print(":"); }

static void meminfo() {
#ifdef _WIN32
  int RAMEND = 0;
  int SP = 0;
  int __brkval = 0;
  int __malloc_heap_start = 0;
  int __malloc_heap_end = 0;
#endif

  separator();
  separator();
  Serial.print(SP - (int)__brkval);
  separator();
  separator();
  Serial.print((int)__malloc_heap_start);
  separator();
  Serial.print((int)__brkval);
  separator();
  Serial.print(SP);
  separator();
  Serial.print(RAMEND);
  separator();
  separator();
}

static void header() {

  _lazySerialInit();
  Serial.print("T");
  meminfo();
}

extern "C" void __ytrace(const char *__file, int __lineno, int32_t param) {
  header();
  Serial.print(__file);
  separator();
  Serial.print(__lineno, DEC);
  separator();
  Serial.println(param, DEC);

  Serial.flush();
  //	while (Serial.available()) Serial.read();
  //	while ( read = Serial.read() ) { if (read != -1) { break; } }
}

extern "C" void __ytracestr(const char *__file, int __lineno,
                            const char *param) {
  header();
  Serial.print(__file);
  separator();
  Serial.print(__lineno, DEC);
  separator();
  Serial.println(param);
  Serial.flush();
  //	while (Serial.available()) Serial.read();
  //	while ( read = Serial.read() ) { if (read != -1) { break; } }
}

// handle diagnostic informations given by assertion and abort program
// execution:
extern "C" void __yassert(const char *__file, int __lineno) {
  // transmit diagnostic informations through serial link.
  header();
  meminfo();
  Serial.println(__file);
  Serial.println(__lineno, DEC);
  Serial.flush();
  do {
    // NOP
  } while (1);
}

#endif
