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

#include "yparser.h"
#include "yassert.h"
#include "ytypes.h"
#include <stdlib.h>

struct yparser {
  yparser_get_char_from_seq_fn getter;
  int sequence_count;
  int cookie;

  enum { STILL_HOPE, NO_MATCH } state;

  /* 0 - given string does not have chance, 1 - there is still a chance */
  uint64_t map;

  /* number of characters parsed*/
  int char_idx;
};

const int YPARSER_RESULT_NO_MATCH_FOUND = -1;
const int YPARSER_RESULT_MORE_DATA_REQUIRED = -2;

/**
This function takes pointer to the parser and a next input character as a
parameter and returns the index of matching sequence or
YPARSER_RESULT_NO_MATCH_FOUND or YPARSER_RESULT_MORE_DATA_REQUIRED
*/
int yparser_next(yparser_t parser, char input) {
  int string_idx;
  uint64_t map_mask = 1;
  int is_there_hope = 0;

  if (parser->state == NO_MATCH) {
    return YPARSER_RESULT_NO_MATCH_FOUND;
  }

  /* iterate over a bitmap looking for 1 */
  for (string_idx = 0; string_idx < parser->sequence_count; string_idx++) {
    if (parser->map & map_mask) {
      char pattern =
          parser->getter(parser->cookie, string_idx, parser->char_idx);
      if (pattern == input) {
        char next_char_from_pattern =
            parser->getter(parser->cookie, string_idx, parser->char_idx + 1);
        if (next_char_from_pattern == -1) {
          /* -1 means end of string, we have a match now, so we can kindly reset
           * the parser*/
          yparser_reset(parser);
          return string_idx;
        } else {
          is_there_hope = 1;
        }
      } else {
        parser->map &= ~map_mask; /* clear the map, we're not going to look
                                     there anymore as it is not a match*/
      }
    }
    map_mask <<= 1;
  }

  if (is_there_hope) {
    parser->char_idx++;
    return YPARSER_RESULT_MORE_DATA_REQUIRED;
  }

  yparser_reset(parser);
  return YPARSER_RESULT_NO_MATCH_FOUND;
}

/**
Returns new parser
*/
yparser_t yparser_alloc(int sequence_count, int cookie,
                        yparser_get_char_from_seq_fn getter) {
  yassert(sequence_count < 64);

  yparser_t result = (struct yparser *)malloc(sizeof(struct yparser));
  yassert(result != NULL);

  result->sequence_count = sequence_count;
  result->getter = getter;
  result->cookie = cookie;
  yparser_reset(result);

  return result;
}

/**
Resets internal state of the parser
*/
void yparser_reset(yparser_t parser) {
  parser->char_idx = 0;
  parser->state = STILL_HOPE;
  parser->map = UINT64_MAX;
}

yerrno_t yparser_free(yparser_t parser) {
  free(parser);
  return Y_OK;
}
