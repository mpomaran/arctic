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
#ifdef RUN_YPARSER_UNIT_TESTS

#include "yassert.h"
#include "yparser.h"
#include <string.h>

static char *tokens[] = {"ATT", "ATI", "320", "OK"};

static int getter(int cookie, int stringIndex, int charIndexWithinString) {
  yassert(cookie == 1);
  yassert(stringIndex < 4);
  char *str = tokens[stringIndex];

  if (charIndexWithinString < (int)strlen(str)) {
    return str[charIndexWithinString];
  } else {
    return -1;
  }
}

void test_yparser_alloc() {
  yparser_t parser;
  parser = yparser_alloc(4, 1, getter);
  yassert(parser != NULL);
  yparser_free(parser);
}

void test_yparser_next() {
  yparser_t parser;
  parser = yparser_alloc(4, 1, getter);
  yassert(parser != NULL);

  int idx = 0;
  yparser_reset(parser);
  char *okSequence = "ATT ATI 320 OK";
  int i;
  for (i = 0; i < (int)strlen(okSequence); i++) {
    if (okSequence[i] == ' ')
      continue;
    ;
    int result = yparser_next(parser, okSequence[i]);
    yassert(result != YPARSER_RESULT_NO_MATCH_FOUND);
    if (result >= 0) {
      yassert(result == idx);
      idx++;
    }
  }
  yassert(idx == 4);

  char *nokSequence1 = "ATT ATI NOK";
  idx = 0;
  yparser_reset(parser);
  for (i = 0; i < (int)strlen(nokSequence1); i++) {
    if (nokSequence1[i] == ' ')
      continue;
    int result = yparser_next(parser, nokSequence1[i]);
    if (result >= 0) {
      yassert(result == idx);
      idx++;
    }
    if (result == YPARSER_RESULT_NO_MATCH_FOUND) {
      break;
    }
  }
  yassert(idx == 2);

  char *nokSequence2 = "ATK";

  for (i = 0; i < (int)strlen(nokSequence2); i++) {
    int result = yparser_next(parser, nokSequence2[i]);
    if (i == 2) {
      yassert(result == YPARSER_RESULT_NO_MATCH_FOUND);
    } else {
      yassert(result == YPARSER_RESULT_MORE_DATA_REQUIRED);
    }
  }
  yparser_free(parser);
}

void yparser_execute_unit_test() {
  test_yparser_alloc();
  test_yparser_next();
}

#endif
