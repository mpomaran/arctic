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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <yassert.h>
#include <ycircbuf.h>

struct ycircbuf {
  uint8_t *buff;
  size_t capacity;
  uint16_t cursor;
  size_t size;
};

/**
\brief Allocates a new buffer with the given capacity. Out of memory errors are
handled only if assert works.

\param capacity of the circ buf to be created
\return created buffer
*/
ycircbuf_t ycircbuf_alloc(size_t capacity) {
  ycircbuf_t result = malloc(sizeof(struct ycircbuf));
  yassert(result != 0);

  result->buff = malloc(capacity);
  yassert(result->buff != 0);

  result->capacity = capacity;
  result->cursor = 0;
  result->size = 0;

  return result;
}

/**
\brief Frees allocated memory for the buffer. Behavior is undefined if buffer
was not previously allocated. \param buf to be used
*/
void ycircbuf_free(ycircbuf_t buf) {
  free(buf->buff);
  free(buf);
}

/**
\brief Returns capacity of the buffer

\param buf to be used
\return capacity of the buffer
*/
size_t ycircbuf_get_capacity(ycircbuf_t buf) { return buf->capacity; }

static void append_byte(ycircbuf_t buf, uint8_t b) {
  yassert(buf->capacity > buf->size);

  size_t write_cursor_pos = buf->cursor + buf->size;
  write_cursor_pos %= buf->capacity;

  buf->buff[write_cursor_pos] = b;
  buf->size++;
}

/**
\brief Adds to the end of the buffer given data. Will not overfill.

\param buf to be used
\param data to be put
\param data_buff_len to be put

\return amount of data put, may be less than data_len if there were not enough
space
*/
size_t ycircbuf_append(ycircbuf_t buf, const uint8_t *data,
                       size_t data_buff_len) {
  size_t data_copied = 0;

  /* TODO: great candidate for optimization */
  while ((buf->size < buf->capacity) && (data_copied < data_buff_len)) {
    append_byte(buf, data[data_copied]);
    data_copied++;
  }

  return data_copied;
}

static uint8_t pop_byte(ycircbuf_t buf) {
  yassert(buf->size);
  uint8_t result = buf->buff[buf->cursor];
  buf->cursor = (buf->cursor + 1) % buf->capacity;
  buf->size--;
  return result;
}

/**
\brief Reads data from the beginning of the buffer and moves the buff pointer.
Non-blocking.

\param buf to be used
\param data where buffer will be copied onto
\param data_buff_len to be put

\return amount of data read, may be less than data_len if there were not enough
data in the buff
*/
size_t ycircbuf_pop(ycircbuf_t buf, uint8_t *data, size_t data_buff_len) {
  size_t data_copied = 0;

  /* TODO: great candidate for optimization */
  while ((buf->size > 0) && (data_copied < data_buff_len)) {
    data[data_copied] = pop_byte(buf);
    data_copied++;
  }

  return data_copied;
}

/**
\brief Gives how much data is currently stored in the buffer

\param buf to be used

\return amount of data stored in the buffer
*/
size_t ycircbuf_size(ycircbuf_t buf) { return buf->size; }
