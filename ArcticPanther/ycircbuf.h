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

#ifndef YCIRCBUF_H_HAS_BEEN_INCLUDED
#define YCIRCBUF_H_HAS_BEEN_INCLUDED

#include <config.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ycircbuf;
typedef struct ycircbuf *ycircbuf_t;

/**
\brief Allocates a new buffer with the given capacity. Out of memory errors are
handled only if assert works.

\param capacity of the circ buf to be created
\return created buffer
*/
ycircbuf_t ycircbuf_alloc(size_t capacity);

/**
\brief Frees allocated memory for the buffer. Behavior is undefined if buffer
was not previously allocated. \param buf to be used
*/
void ycircbuf_free(ycircbuf_t buf);

/**
\brief Returns capacity of the buffer

\param buf to be used
\return capacity of the buffer
*/
size_t ycircbuf_get_capacity(ycircbuf_t buf);

/**
\brief Adds to the end of the buffer given data. Will not overfill.

\param buf to be used
\param data to be put
\param data_buff_len to be put

\return amount of data put, may be less than data_len if there were not enough
space
*/
size_t ycircbuf_append(ycircbuf_t buf, const uint8_t *data,
                       size_t data_buff_len);

/**
\brief Reads data from the beginning of the buffer and moves the buff pointer.
Non-blocking.

\param buf to be used
\param data where buffer will be copied onto
\param data_buff_len to be put

\return amount of data read, may be less than data_len if there were not enough
data in the buff
*/
size_t ycircbuf_pop(ycircbuf_t buf, uint8_t *data, size_t data_buff_len);

/**
\brief Gives how much data is currently stored in the buffer

\param buf to be used

\return amount of data stored in the buffer
*/
size_t ycircbuf_size(ycircbuf_t buf);

#ifdef __cplusplus
}
#endif

#endif
