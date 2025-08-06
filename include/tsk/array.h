#ifndef TSK_ARRAY_H_INCLUDED
#define TSK_ARRAY_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <tsk/type.h>

typedef struct TskArray TskArray;
struct TskArray {
	const TskType *element_type;
	TskAny        *elements;
	TskUSize       length;
	TskUSize       capacity;
};
TskBool       tsk_array_is_valid(const TskArray *array);
TskArray      tsk_array_new(const TskType *element_type);
TskEmpty      tsk_array_drop(TskArray *array);
TskBool       tsk_array_clone(const TskArray *array_1, TskArray *array_2);
TskUSize      tsk_array_length(const TskArray *array);
TskBool       tsk_array_is_empty(const TskArray *array);
TskUSize      tsk_array_capacity(const TskArray *array);
TskUSize      tsk_array_maximum_capacity(const TskArray *array);
TskAny       *tsk_array_get(TskArray *array, TskUSize index);
const TskAny *tsk_array_get_const(const TskArray *array, TskUSize index);
TskAny       *tsk_array_front(TskArray *array);
const TskAny *tsk_array_front_const(const TskArray *array);
TskAny       *tsk_array_back(TskArray *array);
const TskAny *tsk_array_back_const(const TskArray *array);
TskEmpty      tsk_array_swap(TskArray *array, TskUSize index_1, TskUSize index_2);
TskEmpty      tsk_array_clear(TskArray *array);
TskBool       tsk_array_reserve(TskArray *array, TskUSize capacity);
TskBool       tsk_array_reserve_additional(TskArray *array, TskUSize additional);
TskBool       tsk_array_insert(TskArray *array, TskUSize index, TskAny *elements, TskUSize count);
TskEmpty      tsk_array_remove(TskArray *array, TskUSize index, TskAny *elements, TskUSize count);
TskBool       tsk_array_push_front(TskArray *array, TskAny *element);
TskBool       tsk_array_push_back(TskArray *array, TskAny *element);
TskBool       tsk_array_pop_front(TskArray *array, TskAny *element);
TskBool       tsk_array_pop_back(TskArray *array, TskAny *element);
TskBool       tsk_array_linear_search(const TskArray *array, const TskAny *element, TskUSize *index);
TskBool       tsk_array_binary_search(const TskArray *array, const TskAny *element, TskUSize *index);
TskUSize      tsk_array_lower_bound(const TskArray *array, const TskAny *element);
TskUSize      tsk_array_upper_bound(const TskArray *array, const TskAny *element);
TskEmpty      tsk_array_equal_range(const TskArray *array, const TskAny *element, TskUSize *start, TskUSize *end);
TskEmpty      tsk_array_sort(TskArray *array);
TskBool       tsk_array_is_sorted(const TskArray *array);
TskUSize      tsk_array_sorted_until(const TskArray *array);
TskEmpty      tsk_array_partition(TskArray *array, TskBool (*predicate)(const TskAny *element));
TskBool       tsk_array_is_partitioned(const TskArray *array, TskBool (*predicate)(const TskAny *element));
TskUSize      tsk_array_partition_point(TskArray *array, TskBool (*predicate)(const TskAny *element));
TskEmpty      tsk_array_reverse(TskArray *array);
TskOrdering   tsk_array_compare(const TskArray *array_1, const TskArray *array_2);
TskBool       tsk_array_equals(const TskArray *array_1, const TskArray *array_2);
TskEmpty      tsk_array_hash(const TskArray *array, const TskTraitHasher *trait_hasher, TskAny *hasher);

extern const TskType tsk_array_type;

typedef struct TskArrayView TskArrayView;
struct TskArrayView {
	const TskType *element_type;
	TskAny        *elements;
	TskUSize       length;
	TskISize       stride;
};
TskBool      tsk_array_view_is_valid(TskArrayView array_view);
TskArrayView tsk_array_view_slice(TskArrayView array_view, TskUSize start, TskUSize end, TskISize step);
TskUSize     tsk_array_view_length(TskArrayView array_view);
TskBool      tsk_array_view_is_empty(TskArrayView array_view);
TskAny      *tsk_array_view_get(TskArrayView array_view, TskUSize index);
TskAny      *tsk_array_view_front(TskArrayView array_view);
TskAny      *tsk_array_view_back(TskArrayView array_view);
TskEmpty     tsk_array_view_swap(TskArrayView array_view, TskUSize index_1, TskUSize index_2);
TskBool      tsk_array_view_linear_search(TskArrayView array_view, const TskAny *element, TskUSize *index);
TskBool      tsk_array_view_binary_search(TskArrayView array_view, const TskAny *element, TskUSize *index);
TskUSize     tsk_array_view_lower_bound(TskArrayView array_view, const TskAny *element);
TskUSize     tsk_array_view_upper_bound(TskArrayView array_view, const TskAny *element);
TskEmpty     tsk_array_view_equal_range(TskArrayView array_view, const TskAny *element, TskUSize *start, TskUSize *end);
TskEmpty     tsk_array_view_sort(TskArrayView array_view);
TskBool      tsk_array_view_is_sorted(TskArrayView array_view);
TskUSize     tsk_array_view_sorted_until(TskArrayView array_view);
TskEmpty     tsk_array_view_partition(TskArrayView array_view, TskBool (*predicate)(const TskAny *element));
TskBool      tsk_array_view_is_partitioned(TskArrayView array_view, TskBool (*predicate)(const TskAny *element));
TskUSize     tsk_array_view_partition_point(TskArrayView array_view, TskBool (*predicate)(const TskAny *element));
TskEmpty     tsk_array_view_reverse(TskArrayView array_view);
TskOrdering  tsk_array_view_compare(TskArrayView array_view_1, TskArrayView array_view_2);
TskBool      tsk_array_view_equals(TskArrayView array_view_1, TskArrayView array_view_2);
TskEmpty     tsk_array_view_hash(TskArrayView array_view, const TskTraitHasher *trait_hasher, TskAny *hasher);

extern const TskType tsk_array_view_type;

typedef struct TskArrayViewConst TskArrayViewConst;
struct TskArrayViewConst {
	const TskType *element_type;
	const TskAny  *elements;
	TskUSize       length;
	TskISize       stride;
};
TskBool           tsk_array_view_const_is_valid(TskArrayViewConst array_view);
TskArrayViewConst tsk_array_view_const_slice(TskArrayViewConst array_view, TskUSize start, TskUSize end, TskISize step);
TskUSize          tsk_array_view_const_length(TskArrayViewConst array_view);
TskBool           tsk_array_view_const_is_empty(TskArrayViewConst array_view);
const TskAny     *tsk_array_view_const_get(TskArrayViewConst array_view, TskUSize index);
const TskAny     *tsk_array_view_const_front(TskArrayViewConst array_view);
const TskAny     *tsk_array_view_const_back(TskArrayViewConst array_view);
TskBool           tsk_array_view_const_linear_search(TskArrayViewConst array_view, const TskAny *element, TskUSize *index);
TskBool           tsk_array_view_const_binary_search(TskArrayViewConst array_view, const TskAny *element, TskUSize *index);
TskUSize          tsk_array_view_const_lower_bound(TskArrayViewConst array_view, const TskAny *element);
TskUSize          tsk_array_view_const_upper_bound(TskArrayViewConst array_view, const TskAny *element);
TskEmpty          tsk_array_view_const_equal_range(TskArrayViewConst array_view, const TskAny *element, TskUSize *start, TskUSize *end);
TskBool           tsk_array_view_const_is_sorted(TskArrayViewConst array_view);
TskUSize          tsk_array_view_const_sorted_until(TskArrayViewConst array_view);
TskBool           tsk_array_view_const_is_partitioned(TskArrayViewConst array_view, TskBool (*predicate)(const TskAny *element));
TskUSize          tsk_array_view_const_partition_point(TskArrayViewConst array_view, TskBool (*predicate)(const TskAny *element));
TskOrdering       tsk_array_view_const_compare(TskArrayViewConst array_view_1, TskArrayViewConst array_view_2);
TskBool           tsk_array_view_const_equals(TskArrayViewConst array_view_1, TskArrayViewConst array_view_2);
TskEmpty          tsk_array_view_const_hash(TskArrayViewConst array_view, const TskTraitHasher *trait_hasher, TskAny *hasher);

extern const TskType tsk_array_view_const_type;

TskArrayViewConst tsk_array_view_to_const(TskArrayView array_view);

TskArrayView      tsk_array_to_view(TskArray *array);
TskArrayViewConst tsk_array_to_view_const(const TskArray *array);

static inline TskArrayView tsk_array_view_identity(TskArrayView arrary_view) {
	return arrary_view;
}
static inline TskArrayViewConst tsk_array_view_const_identity(TskArrayViewConst arrary_view) {
	return arrary_view;
}
#define TSK_ARRAY_VIEW_AUTO(value)                      \
	_Generic(                                             \
	    (value),                                          \
	    TskArrayView: tsk_array_view_identity,            \
	    TskArrayViewConst: tsk_array_view_const_identity, \
	    TskArray *: tsk_array_to_view,                    \
	    const TskArray *: tsk_array_to_view_const,        \
	)(value)
#define TSK_ARRAY_VIEW_CONST_AUTO(value)                \
	_Generic(                                             \
	    (value),                                          \
	    TskArrayView: tsk_array_view_to_const,            \
	    TskArrayViewConst: tsk_array_view_const_identity, \
	    TskArray *: tsk_array_to_view_const,              \
	    const TskArray *: tsk_array_to_view_const,        \
	)(value)

#ifdef __cplusplus
}
#endif

#endif // TSK_ARRAY_H_INCLUDED
