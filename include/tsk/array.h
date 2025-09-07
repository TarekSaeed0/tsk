#ifndef TSK_ARRAY_H_INCLUDED
#define TSK_ARRAY_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <tsk/trait/comparable.h>
#include <tsk/type.h>

typedef struct TskArray TskArray;
struct TskArray {
	TskAny  *elements;
	TskUSize length;
	TskUSize capacity;
};
TskBoolean     tsk_array_is_valid(const TskType *array_type, const TskArray *array);
TskArray       tsk_array_new(const TskType *array_type);
TskEmpty       tsk_array_drop(const TskType *array_type, TskArray *array);
TskBoolean     tsk_array_clone(const TskType *array_type, const TskArray *array_1, TskArray *array_2);
const TskType *tsk_array_element_type(const TskType *array_type);
TskAny        *tsk_array_elements(const TskType *array_type, TskArray *array);
const TskAny  *tsk_array_elements_const(const TskType *array_type, const TskArray *array);
TskUSize       tsk_array_length(const TskType *array_type, const TskArray *array);
TskBoolean     tsk_array_is_empty(const TskType *array_type, const TskArray *array);
TskUSize       tsk_array_capacity(const TskType *array_type, const TskArray *array);
TskUSize       tsk_array_maximum_capacity(const TskType *array_type);
TskAny        *tsk_array_get(const TskType *array_type, TskArray *array, TskUSize index);
const TskAny  *tsk_array_get_const(const TskType *array_type, const TskArray *array, TskUSize index);
TskAny        *tsk_array_front(const TskType *array_type, TskArray *array);
const TskAny  *tsk_array_front_const(const TskType *array_type, const TskArray *array);
TskAny        *tsk_array_back(const TskType *array_type, TskArray *array);
const TskAny  *tsk_array_back_const(const TskType *array_type, const TskArray *array);
TskEmpty       tsk_array_clear(const TskType *array_type, TskArray *array);
TskBoolean     tsk_array_reserve(const TskType *array_type, TskArray *array, TskUSize capacity);
TskBoolean     tsk_array_reserve_additional(const TskType *array_type, TskArray *array, TskUSize additional);
TskBoolean     tsk_array_insert(const TskType *array_type, TskArray *array, TskUSize index, TskAny *elements, TskUSize count);
TskEmpty       tsk_array_remove(const TskType *array_type, TskArray *array, TskUSize index, TskAny *elements, TskUSize count);
TskBoolean     tsk_array_push_front(const TskType *array_type, TskArray *array, TskAny *element);
TskBoolean     tsk_array_push_back(const TskType *array_type, TskArray *array, TskAny *element);
TskBoolean     tsk_array_pop_front(const TskType *array_type, TskArray *array, TskAny *element);
TskBoolean     tsk_array_pop_back(const TskType *array_type, TskArray *array, TskAny *element);
TskOrdering    tsk_array_compare(const TskType *array_type, const TskArray *array_1, const TskArray *array_2);
TskBoolean     tsk_array_equals(const TskType *array_type, const TskArray *array_1, const TskArray *array_2);
TskEmpty       tsk_array_hash(const TskType *array_type, const TskArray *array, const TskType *hasher_type, TskAny *hasher);

TskBoolean     tsk_array_type_is_valid(const TskType *array_type);
const TskType *tsk_array_type(const TskType *element_type);

typedef struct TskArrayView TskArrayView;
struct TskArrayView {
	const TskType *element_type;
	TskAny        *elements;
	TskUSize       length;
	TskISize       stride;
};
TskBoolean     tsk_array_view_is_valid(const TskType *array_view_type, TskArrayView array_view);
TskArrayView   tsk_array_view_new(const TskType *array_view_type, TskAny *elements, TskUSize length, TskISize stride);
TskArrayView   tsk_array_view_slice(const TskType *array_view_type, TskArrayView array_view, TskUSize start, TskUSize end, TskISize step);
const TskType *tsk_array_view_element_type(const TskType *array_view_type);
TskAny        *tsk_array_view_elements(const TskType *array_view_type, TskArrayView array_view);
TskUSize       tsk_array_view_length(const TskType *array_view_type, TskArrayView array_view);
TskBoolean     tsk_array_view_is_empty(const TskType *array_view_type, TskArrayView array_view);
TskISize       tsk_array_view_stride(const TskType *array_view_type, TskArrayView array_view);
TskAny        *tsk_array_view_get(const TskType *array_view_type, TskArrayView array_view, TskUSize index);
TskAny        *tsk_array_view_front(const TskType *array_view_type, TskArrayView array_view);
TskAny        *tsk_array_view_back(const TskType *array_view_type, TskArrayView array_view);
TskEmpty       tsk_array_view_swap(const TskType *array_view_type, TskArrayView array_view, TskUSize index_1, TskUSize index_2);
TskEmpty       tsk_array_view_sort(const TskType *array_view_type, TskArrayView array_view);
TskEmpty       tsk_array_view_partition(const TskType *array_view_type, TskArrayView array_view, TskBoolean (*predicate)(const TskAny *element));
TskEmpty       tsk_array_view_reverse(const TskType *array_view_type, TskArrayView array_view);
TskOrdering    tsk_array_view_compare(const TskType *array_view_type, TskArrayView array_view_1, TskArrayView array_view_2);
TskBoolean     tsk_array_view_equals(const TskType *array_view_type, TskArrayView array_view_1, TskArrayView array_view_2);
TskEmpty       tsk_array_view_hash(const TskType *array_view_type, TskArrayView array_view, const TskType *hasher_type, TskAny *hasher);

TskBoolean     tsk_array_view_type_is_valid(const TskType *array_view_type);
const TskType *tsk_array_view_type(const TskType *element_type);

TskArrayView tsk_array_view(const TskType *array_type, TskArray *array);

typedef struct TskArrayViewConst TskArrayViewConst;
struct TskArrayViewConst {
	const TskAny *elements;
	TskUSize      length;
	TskISize      stride;
};
TskBoolean        tsk_array_view_const_is_valid(const TskType *array_view_type, TskArrayViewConst array_view);
TskArrayViewConst tsk_array_view_const_new(const TskType *array_view_type, const TskAny *elements, TskUSize length, TskISize stride);
TskArrayViewConst tsk_array_view_const_slice(const TskType *array_view_type, TskArrayViewConst array_view, TskUSize start, TskUSize end, TskISize step);
const TskType    *tsk_array_view_const_element_type(const TskType *array_view_type);
const TskAny     *tsk_array_view_const_elements(const TskType *array_view_type, TskArrayViewConst array_view);
TskUSize          tsk_array_view_const_length(const TskType *array_view_type, TskArrayViewConst array_view);
TskBoolean        tsk_array_view_const_is_empty(const TskType *array_view_type, TskArrayViewConst array_view);
TskISize          tsk_array_view_const_stride(const TskType *array_view_type, TskArrayViewConst array_view);
const TskAny     *tsk_array_view_const_get(const TskType *array_view_type, TskArrayViewConst array_view, TskUSize index);
const TskAny     *tsk_array_view_const_front(const TskType *array_view_type, TskArrayViewConst array_view);
const TskAny     *tsk_array_view_const_back(const TskType *array_view_type, TskArrayViewConst array_view);
TskBoolean        tsk_array_view_const_linear_search(const TskType *array_view_type, TskArrayViewConst array_view, const TskAny *element, TskUSize *index);
TskBoolean        tsk_array_view_const_binary_search(const TskType *array_view_type, TskArrayViewConst array_view, const TskAny *element, TskUSize *index);
TskUSize          tsk_array_view_const_lower_bound(const TskType *array_view_type, TskArrayViewConst array_view, const TskAny *element);
TskUSize          tsk_array_view_const_upper_bound(const TskType *array_view_type, TskArrayViewConst array_view, const TskAny *element);
TskEmpty          tsk_array_view_const_equal_range(const TskType *array_view_type, TskArrayViewConst array_view, const TskAny *element, TskUSize *start, TskUSize *end);
TskBoolean        tsk_array_view_const_is_sorted(const TskType *array_view_type, TskArrayViewConst array_view);
TskUSize          tsk_array_view_const_sorted_until(const TskType *array_view_type, TskArrayViewConst array_view);
TskBoolean        tsk_array_view_const_is_partitioned(const TskType *array_view_type, TskArrayViewConst array_view, TskBoolean (*predicate)(const TskAny *element));
TskUSize          tsk_array_view_const_partition_point(const TskType *array_view_type, TskArrayViewConst array_view, TskBoolean (*predicate)(const TskAny *element));
TskOrdering       tsk_array_view_const_compare(const TskType *array_view_type, TskArrayViewConst array_view_1, TskArrayViewConst array_view_2);
TskBoolean        tsk_array_view_const_equals(const TskType *array_view_type, TskArrayViewConst array_view_1, TskArrayViewConst array_view_2);
TskEmpty          tsk_array_view_const_hash(const TskType *array_view_type, TskArrayViewConst array_view, const TskType *hasher_type, TskAny *hasher);

TskBoolean     tsk_array_view_const_type_is_valid(const TskType *array_view_type);
const TskType *tsk_array_view_const_type(const TskType *element_type);

TskArrayViewConst tsk_array_view_const(const TskType *array_type, const TskArray *array);
TskArrayViewConst tsk_array_view_as_const(const TskType *array_view_type, TskArrayView array_view);

typedef struct TskArrayIterator TskArrayIterator;
struct TskArrayIterator {
	TskAny  *elements;
	TskUSize length;
	TskISize stride;
};
TskBoolean     tsk_array_iterator_is_valid(const TskType *array_iterator_type, const TskArrayIterator *array_iterator);
const TskType *tsk_array_iterator_element_type(const TskType *array_iterator_type);
const TskType *tsk_array_iterator_item_type(const TskType *array_iterator_type);
TskBoolean     tsk_array_iterator_next(const TskType *array_iterator_type, TskArrayIterator *array_iterator, TskAny *item);

TskBoolean     tsk_array_iterator_type_is_valid(const TskType *array_iterator_type);
const TskType *tsk_array_iterator_type(const TskType *element_type);

TskArrayIterator tsk_array_iterator(const TskType *array_type, TskArray *array);
TskArrayIterator tsk_array_view_iterator(const TskType *array_view_type, TskArrayView array_view);

typedef struct TskArrayIteratorConst TskArrayIteratorConst;
struct TskArrayIteratorConst {
	const TskAny *elements;
	TskUSize      length;
	TskISize      stride;
};
TskBoolean     tsk_array_iterator_const_is_valid(const TskType *array_iterator_type, const TskArrayIteratorConst *array_iterator);
const TskType *tsk_array_iterator_const_element_type(const TskType *array_iterator_type);
const TskType *tsk_array_iterator_const_item_type(const TskType *array_iterator_type);
TskBoolean     tsk_array_iterator_const_next(const TskType *array_iterator_type, TskArrayIteratorConst *array_iterator, TskAny *item);

TskBoolean     tsk_array_iterator_const_type_is_valid(const TskType *array_iterator_type);
const TskType *tsk_array_iterator_const_type(const TskType *element_type);

TskArrayIteratorConst tsk_array_iterator_const(const TskType *array_type, const TskArray *array);
TskArrayIteratorConst tsk_array_view_const_iterator(const TskType *array_view_type, TskArrayViewConst array_view);

#ifdef __cplusplus
}
#endif

#endif // TSK_ARRAY_H_INCLUDED
