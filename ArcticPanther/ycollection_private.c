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

#include "ycollection_private.h"

yerrno_t ycollection_free(ycollection_t collection) {
  return collection->handlers.free_fn_ptr(collection);
}

yerrno_t ycollection_push(ycollection_t collection, void *value_ptr) {
  return collection->handlers.push_fn_ptr(collection, value_ptr);
}

yerrno_t ycollection_get(ycollection_t collection, size_t idx, void *result) {
  return collection->handlers.get_fn_ptr(collection, idx, result);
}

yerrno_t ycollection_remove(ycollection_t collection, size_t idx) {
  return collection->handlers.remove_fn_ptr(collection, idx);
}

size_t ycollection_size(ycollection_t collection) { return collection->size; }
