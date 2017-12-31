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

#ifndef YCOLLECTION_H_HAS_BEEN_INCLUDED
#define YCOLLECTION_H_HAS_BEEN_INCLUDED

#include "ytypes.h"

struct ycollection;
typedef struct ycollection *ycollection_t;

/**
\brief creates a new list allocatig a memory for it

\param elem_size size of a list element
\param free_fn function to be called when list is freed; not used if NULL is
passed \param buff_len indicates amount of bytes to be send

\return created list or NULL in case of errors
*/
ycollection_t ycollection_list_alloc(uint16_t elem_size,
                                     void (*free_fn)(void *));

yerrno_t ycollection_free(ycollection_t collection);

/**
\brief allocates new memory and copies data provided by value_ptr to the end of
the collection

\param collection specifies on which collection operation should be performed
\param value_ptr pointer to the buffer from where data is going to be copied

\return status of the operation
*/
yerrno_t ycollection_push(ycollection_t collection, void *value_ptr);

/**
\brief removes given element from the collection

\param collection specifies on which collection operation should be performed
\param idx index of the element to be accessed
\param result pointer to the buffer where data is going to be copied

\return status of the operation
*/
yerrno_t ycollection_get(ycollection_t collection, size_t idx, void *result);

/**
\brief removes given element from the collection

\param collection specifies on which collection operation should be performed
\param idx index of the element to be removed

\return status of the operation
*/
yerrno_t ycollection_remove(ycollection_t collection, size_t idx);

/**
\brief gives the size of the collection

\param collection specifies on which collection operation should be performed

\return the size of the colletion
*/
size_t ycollection_size(ycollection_t collection);

#endif
