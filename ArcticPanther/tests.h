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

#ifndef TESTS_H_INCLUDED
#define TESTS_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define RUN_YPARSER_UNIT_TESTS
#define RUN_YSERIAL_UNIT_TESTS
#define RUN_YGSMMODEM_UNIT_TESTS
#define RUN_YTIMER_UNIT_TESTS
#define RUN_YCIRCBUF_UNIT_TESTS
#define RUN_YLIST_UNIT_TESTS
#define RUN_YSIM900_EMU_UNIT_TESTS
#define RUN_YSTRING_UNIT_TESTS
#define RUN_YMESSAGE_UNIT_TESTS
#define RUN_UBASIC_UNIT_TESTS
#else
#ifdef ARDUINO_ARCH_AVR
//#define RUN_YPARSER_UNIT_TESTS
//#define RUN_YSHA256_UNIT_TESTS
//#define RUN_YSERIAL_UNIT_TESTS
//#define RUN_YGSMMODEM_UNIT_TESTS
//#define RUN_YTIMER_UNIT_TESTS
//#define RUN_YCIRCBUF_UNIT_TESTS
//#define RUN_YLIST_UNIT_TESTS
//#define RUN_YSIM900_EMU_UNIT_TESTS
//#define RUN_YSTRING_UNIT_TESTS
//#define RUN_YMESSAGE_UNIT_TESTS
//#define RUN_UBASIC_UNIT_TESTS
#else
#error Uknown architecture
#endif
#endif

#ifndef NDEBUG
void yparser_execute_unit_test(void);
void yserial_execute_unit_tests(void);
void ygsmmodem_execute_unit_tests(void);
void ytimer_execute_unit_tests(void);
void ycircbuf_execute_unit_test(void);
void ylist_test_suite(void);
void ysim900_emu_test_suite(void);
void ystring_unit_tests(void);
void ymessage_unit_tests();
void ubasic_unit_tests(void);
#endif

void y_unit_test(void);

#ifdef __cplusplus
}
#endif

#endif
