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

#ifdef RUN_YLIST_UNIT_TESTS

#include "yassert.h"
#include "ytypes.h"

#include "ycollection.h"

#ifdef _WIN32
#pragma warning(disable : 4022)
#endif

/*
Scenario: initializing and destroying non empty list

Given
 populated list
When
 I invoke constructor
Then
 I should get handle to the array which allows me to clean up after myself
*/
void should_allow_destroying_non_empty_list() {
  ycollection_t list;
  uint32_t static_value;
  uint32_t i;

  static_value = 1;

  for (i = 0; i < 1000; i++) {
    list = ycollection_list_alloc(sizeof(uint8_t), NULL);
    ycollection_free(list);

    list = ycollection_list_alloc(sizeof(uint8_t), NULL);
    ycollection_push(list, static_value);
    ycollection_push(list, static_value);
    ycollection_free(list);

    list = ycollection_list_alloc(sizeof(uint8_t *), free);
    uint32_t *dynamic_value = malloc(sizeof(uint8_t));
    ycollection_push(list, dynamic_value);
    ycollection_free(list);

    list = ycollection_list_alloc(sizeof(uint8_t), NULL);
    ycollection_push(list, static_value);
    ycollection_push(list, static_value);
    ycollection_push(list, static_value);
    ycollection_free(list);
  }
}

/*
Scenario: getting given element from the list

Given
 a list with 1, 2 or 3 elements
When
 I will try to retreive element
Then
 I will get it
*/
void should_allow_getting_element_from_list() {
  ycollection_t l1 = ycollection_list_alloc(sizeof(uint8_t), NULL);
  ycollection_t l2 = ycollection_list_alloc(sizeof(uint8_t), NULL);
  ycollection_t l3 = ycollection_list_alloc(sizeof(uint8_t *), free);
  uint8_t v;
  uint8_t *v1, *v2, *v3;
  uint8_t *dyn_v;

  ycollection_push(l1, 0);
  ycollection_push(l2, 0);
  ycollection_push(l2, 1);

  v1 = malloc(sizeof(uint8_t));
  v2 = malloc(sizeof(uint8_t));
  v3 = malloc(sizeof(uint8_t));
  *v1 = 1;
  *v2 = 2;
  *v3 = 3;
  ycollection_push(l3, v1);
  ycollection_push(l3, v2);
  ycollection_push(l3, v3);

  yassert(Y_OK == ycollection_get(l1, 0, &v));
  yassert(v == 0);
  yassert(Y_OK == ycollection_get(l2, 0, &v));
  yassert(v == 0);
  yassert(Y_OK == ycollection_get(l2, 1, &v));
  yassert(v == 1);

  yassert(Y_OK == ycollection_get(l3, 0, &dyn_v));
  yassert(*dyn_v == 1);
  yassert(Y_OK == ycollection_get(l3, 1, &dyn_v));
  yassert(*dyn_v == 2);
  yassert(Y_OK == ycollection_get(l3, 2, &dyn_v));
  yassert(*dyn_v == 3);
  yassert(Y_INDEX_OUT_OF_BOUNDS == ycollection_get(l3, 3, &dyn_v));
  yassert(*dyn_v == 3);

  ycollection_free(l1);
  ycollection_free(l2);
  ycollection_free(l3);
}

/*
Scenario: Given a list user should be able to remove any element from it

Given
 an arbitrary size list
When
 user removes element from the list
Then
 size of the list decreases or stays 0 if list is empty
 element is removed and the list is still linked in chain
 all element are accessible after the removal
*/
void should_allow_removing_element_from_list() {
  uint8_t value;

  ycollection_t list = ycollection_list_alloc(sizeof(uint8_t), NULL);
  yassert(Y_INDEX_OUT_OF_BOUNDS == ycollection_remove(list, 0));
  ycollection_free(list);

  list = ycollection_list_alloc(sizeof(uint8_t), NULL);
  yassert(ycollection_push(list, 0) == Y_OK);
  yassert(ycollection_size(list) == 1);
  yassert(ycollection_remove(list, 0) == Y_OK);
  yassert(ycollection_size(list) == 0);
  ycollection_free(list);

  list = ycollection_list_alloc(sizeof(uint8_t), NULL);
  yassert(ycollection_push(list, 1) == Y_OK);
  yassert(ycollection_push(list, 2) == Y_OK);
  yassert(ycollection_size(list) == 2);
  yassert(ycollection_remove(list, 0) == Y_OK);
  yassert(ycollection_get(list, 0, &value) == Y_OK);
  yassert(value == 2);
  yassert(ycollection_size(list) == 1);
  ycollection_free(list);

  list = ycollection_list_alloc(sizeof(uint8_t), NULL);
  yassert(ycollection_push(list, 1) == Y_OK);
  yassert(ycollection_push(list, 2) == Y_OK);
  yassert(ycollection_size(list) == 2);
  yassert(ycollection_remove(list, 1) == Y_OK);
  yassert(ycollection_get(list, 0, &value) == Y_OK);
  yassert(value == 1);
  yassert(ycollection_size(list) == 1);
  ycollection_free(list);

  list = ycollection_list_alloc(sizeof(uint8_t), NULL);
  yassert(ycollection_push(list, 1) == Y_OK);
  yassert(ycollection_push(list, 2) == Y_OK);
  yassert(ycollection_push(list, 3) == Y_OK);
  yassert(ycollection_size(list) == 3);
  yassert(ycollection_remove(list, 0) == Y_OK);
  yassert(ycollection_get(list, 0, &value) == Y_OK);
  yassert(value == 2);
  yassert(ycollection_get(list, 1, &value) == Y_OK);
  yassert(value == 3);
  yassert(ycollection_size(list) == 2);
  ycollection_free(list);

  list = ycollection_list_alloc(sizeof(uint8_t), NULL);
  yassert(ycollection_push(list, 1) == Y_OK);
  yassert(ycollection_push(list, 2) == Y_OK);
  yassert(ycollection_push(list, 3) == Y_OK);
  yassert(ycollection_size(list) == 3);
  yassert(ycollection_remove(list, 1) == Y_OK);
  yassert(ycollection_get(list, 0, &value) == Y_OK);
  yassert(value == 1);
  yassert(ycollection_get(list, 1, &value) == Y_OK);
  yassert(value == 3);
  yassert(ycollection_size(list) == 2);
  ycollection_free(list);

  list = ycollection_list_alloc(sizeof(uint8_t), NULL);
  yassert(ycollection_push(list, 1) == Y_OK);
  yassert(ycollection_push(list, 2) == Y_OK);
  yassert(ycollection_push(list, 3) == Y_OK);
  yassert(ycollection_size(list) == 3);
  yassert(ycollection_remove(list, 2) == Y_OK);
  yassert(ycollection_get(list, 0, &value) == Y_OK);
  yassert(value == 1);
  yassert(ycollection_get(list, 1, &value) == Y_OK);
  yassert(value == 2);
  yassert(ycollection_size(list) == 2);
  ycollection_free(list);
}

void ylist_test_suite() {
#ifdef _WIN32
  uint32_t i;

  for (i = 0; i < 30; i++) {
#endif
    should_allow_destroying_non_empty_list();
    should_allow_getting_element_from_list();
    should_allow_removing_element_from_list();
#ifdef _WIN32
  }
#endif
}

#endif
