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

#ifndef YS_PARSER_H_HAS_BEEN_INCLUDED
#define YS_PARSER_H_HAS_BEEN_INCLUDED

#include "ytypes.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct yparser;

typedef struct yparser *yparser_t;

/** this type defines function used to get required sequence */
/** it has 3 paramerers - cookie, index of string and index of chanracter in
 * string - should return -1 if index out of bound */
typedef int (*yparser_get_char_from_seq_fn)(int cookie, int stringIndex,
                                            int charIndexWithinString);

extern const int YPARSER_RESULT_NO_MATCH_FOUND;
extern const int YPARSER_RESULT_MORE_DATA_REQUIRED;

/**
This function takes pointer to the parser and a next input character as a
parameter and returns the index of matching sequence or
YPARSER_RESULT_NO_MATCH_FOUND or YPARSER_RESULT_MORE_DATA_REQUIRED if match is
found or YPARSER_RESULT_NO_MATCH_FOUND the parser resets automatically
*/
int yparser_next(yparser_t parser, char next_byte);

/**
Returns new parser
*/
yparser_t yparser_alloc(int sequence_count, int cookie,
                        yparser_get_char_from_seq_fn getter);

/**
Deallocates the memory taken by parser
*/
yerrno_t yparser_free(yparser_t parser);

/**
Resets internal state of the parser
*/
void yparser_reset(yparser_t parser);

#ifdef __cplusplus
}
#endif

#endif /* of YS_PARSER_H_HAS_BEEN_INCLUDED */
