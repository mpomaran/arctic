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

#include "tests.h"

void y_unit_test() {
#ifdef RUN_YSTRING_UNIT_TESTS
  ystring_unit_tests();
#endif

#ifdef RUN_UBASIC_UNIT_TESTS
  ubasic_unit_tests();
#endif

#ifdef RUN_YPARSER_UNIT_TESTS
  yparser_execute_unit_test();
#endif

#ifdef RUN_YSERIAL_UNIT_TESTS
  yserial_execute_unit_tests();
#endif

#ifdef RUN_YGSMMODEM_UNIT_TESTS
  ygsmmodem_execute_unit_tests();
#endif

#ifdef RUN_YTIMER_UNIT_TESTS
  ytimer_execute_unit_tests();
#endif

#ifdef RUN_YCIRCBUF_UNIT_TESTS
  ycircbuf_execute_unit_test();
#endif

#ifdef RUN_YLIST_UNIT_TESTS
  ylist_test_suite();
#endif

#ifdef RUN_YSIM900_EMU_UNIT_TESTS
  ysim900_emu_test_suite();
#endif

#ifdef RUN_YMESSAGE_UNIT_TESTS
  ymessage_unit_tests();
#endif
}
