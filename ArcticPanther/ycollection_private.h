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

/* THIS FILE SHOULD NEVER BE INCLUDED BY USER */

#ifndef YCOLLECTION_PRIVATE_H_HAS_BEEN_INCLUDED
#define YCOLLECTION_PRIVATE_H_HAS_BEEN_INCLUDED

#include "ycollection.h"
#include "ytypes.h"

struct ycollection_handlers_private {
  yerrno_t (*push_fn_ptr)(ycollection_t, void *);
  yerrno_t (*free_fn_ptr)(ycollection_t);
  yerrno_t (*get_fn_ptr)(ycollection_t collection, size_t idx, void *result);
  yerrno_t (*remove_fn_ptr)(ycollection_t collection, size_t idx);
};

struct ycollection {
  struct ycollection_handlers_private handlers;
  size_t size;
  void *data;
};

#endif
