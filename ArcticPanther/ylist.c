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

#include "yassert.h"
#include "ycollection.h"
#include "ycollection_private.h"
#include "ytypes.h"

#include <memory.h>

yerrno_t ylist_push(ycollection_t collection, void *);
yerrno_t ylist_free(ycollection_t collection);
yerrno_t ylist_get_element_value(ycollection_t collection, size_t idx,
                                 void *result);
yerrno_t ylist_remove(ycollection_t collection, size_t idx);

static struct ycollection_handlers_private ylist_handlers = {
    ylist_push, ylist_free, ylist_get_element_value, ylist_remove};

struct ylist_node;
typedef struct ylist_node *ylist_node_t;
struct ylist_node {
  void *data;
  ylist_node_t next;
};

struct ylist {
  void (*free_fn)(void *);
  ylist_node_t head;
  size_t elem_size;
};
typedef struct ylist *ylist_t;

ylist_t ylist_get_list_from_collection(ycollection_t collection) {
  yassert(collection != NULL);
  return ((ylist_t)collection->data);
}

ycollection_t ycollection_list_alloc(uint16_t elem_size,
                                     void (*free_fn)(void *)) {
  if (elem_size == 0) {
    return NULL;
  }

  ycollection_t result = malloc(sizeof(struct ycollection));
  if (result == NULL) {
    return NULL;
  }

  ylist_t list = malloc(sizeof(struct ylist));
  list->head = NULL;
  list->elem_size = elem_size;
  list->free_fn = free_fn;
  result->size = 0;

  result->handlers = ylist_handlers;
  result->data = list;

  return result;
}

ylist_node_t ylist_alloc_new_node(size_t data_size, void *data) {
  ylist_node_t new_node = malloc(sizeof(struct ylist_node));

  if (new_node == NULL) {
    return NULL;
  }

  yassert(data_size <= sizeof(void *));
  memcpy(&(new_node->data), &data, data_size);
  new_node->next = NULL;

  return new_node;
}

ylist_node_t ylist_get_node(ycollection_t collection, size_t idx) {
  ylist_t list = ylist_get_list_from_collection(collection);

  // if idx is not within the list range
  if (idx + 1 > ycollection_size(collection)) {
    return NULL;
  }

  ylist_node_t node = list->head;

  for (; idx > 0; idx--) {
    node = node->next;
  }

  return node;
}

ylist_node_t ylist_get_tail(ycollection_t collection) {
  ylist_t list = ylist_get_list_from_collection(collection);
  ylist_node_t tail = NULL;
  size_t size = ycollection_size(collection);
  if (size > 0) {
    tail = ylist_get_node(collection, size - 1);
  }

  return tail;
}

yerrno_t ylist_push(ycollection_t collection, void *data) {
  ylist_t list = ylist_get_list_from_collection(collection);
  ylist_node_t new_node = ylist_alloc_new_node(list->elem_size, data);
  ylist_node_t tail = ylist_get_tail(collection);

  if (tail == NULL) {
    list->head = new_node;
  } else {
    tail->next = new_node;
  }
  collection->size++;

  return Y_OK;
}

static void nop_free(void *ptr) { /* NOP */
}

yerrno_t ylist_free(ycollection_t collection) {
  ylist_t list = ylist_get_list_from_collection(collection);

  void (*free_fn)(void *) = ((ylist_t)collection->data)->free_fn;
  if (free_fn == NULL) {
    free_fn = nop_free;
  }

  ylist_node_t node = ylist_get_node(collection, 0);

  while (node != NULL) {
    ylist_node_t next_node = node->next;
    yassert(next_node != node);
    free_fn(node->data);

    free(node);
    node = next_node;
  }
  free(list);
  free(collection);

  return Y_OK;
}

yerrno_t ylist_get_element_value(ycollection_t collection, size_t idx,
                                 void *result) {
  ylist_node_t node = ylist_get_node(collection, idx);
  yerrno_t retval;

  if (node == NULL) {
    retval = Y_INDEX_OUT_OF_BOUNDS;
  } else {
    ylist_t list = ylist_get_list_from_collection(collection);
    memcpy(result, &node->data, list->elem_size);
    retval = Y_OK;
  }

  return retval;
}

yerrno_t ylist_remove(ycollection_t collection, size_t idx) {
  size_t initial_size = ycollection_size(collection);
  if (initial_size == 0) {
    return Y_INDEX_OUT_OF_BOUNDS;
  }

  ylist_t list = ylist_get_list_from_collection(collection);
  ylist_node_t prev;
  ylist_node_t next;
  ylist_node_t to_be_removed = ylist_get_node(collection, idx);

  if (idx == 0) {
    prev = NULL;
  } else {
    prev = ylist_get_node(collection, idx - 1);
  }

  if (idx == initial_size - 1) {
    next = NULL;
  } else {
    next = ylist_get_node(collection, idx + 1);
  }

  if (prev == NULL) {
    list->head = next;
  } else {
    prev->next = next;
  }

  if (to_be_removed != NULL) {
    void (*free_fn)(void *) = ((ylist_t)collection->data)->free_fn;
    if (free_fn == NULL) {
      free_fn = nop_free;
    }

    free_fn(to_be_removed->data);
    free(to_be_removed);
  }

  collection->size--;

  return Y_OK;
}
