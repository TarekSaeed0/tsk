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

/* typedef struct TskArrayView TskArrayView;
struct TskArrayView {
  const TskType *element_type;
  TskAny        *elements;
  TskUSize       length;
  TskISize       stride;
};
TskBoolean        tsk_array_view_is_valid(TskArrayView array_view);
TskArrayView   tsk_array_view_slice(TskArrayView array_view, TskUSize start, TskUSize end, TskISize step);
const TskType *tsk_array_view_element_type(TskArrayView array_view);
TskUSize       tsk_array_view_length(TskArrayView array_view);
TskBoolean        tsk_array_view_is_empty(TskArrayView array_view);
TskISize       tsk_array_view_stride(TskArrayView array_view);
TskAny        *tsk_array_view_get(TskArrayView array_view, TskUSize index);
TskAny        *tsk_array_view_front(TskArrayView array_view);
TskAny        *tsk_array_view_back(TskArrayView array_view);
TskEmpty       tsk_array_view_swap(TskArrayView array_view, TskUSize index_1, TskUSize index_2);
TskBoolean        tsk_array_view_linear_search(TskArrayView array_view, const TskAny *element, TskUSize *index);
TskBoolean        tsk_array_view_binary_search(TskArrayView array_view, const TskAny *element, TskUSize *index);
TskUSize       tsk_array_view_lower_bound(TskArrayView array_view, const TskAny *element);
TskUSize       tsk_array_view_upper_bound(TskArrayView array_view, const TskAny *element);
TskEmpty       tsk_array_view_equal_range(TskArrayView array_view, const TskAny *element, TskUSize *start, TskUSize *end);
TskEmpty       tsk_array_view_sort(TskArrayView array_view);
TskBoolean        tsk_array_view_is_sorted(TskArrayView array_view);
TskUSize       tsk_array_view_sorted_until(TskArrayView array_view);
TskEmpty       tsk_array_view_partition(TskArrayView array_view, TskBoolean (*predicate)(const TskAny *element));
TskBoolean        tsk_array_view_is_partitioned(TskArrayView array_view, TskBoolean (*predicate)(const TskAny *element));
TskUSize       tsk_array_view_partition_point(TskArrayView array_view, TskBoolean (*predicate)(const TskAny *element));
TskEmpty       tsk_array_view_reverse(TskArrayView array_view);
TskOrdering    tsk_array_view_compare(TskArrayView array_view_1, TskArrayView array_view_2);
TskBoolean        tsk_array_view_equals(TskArrayView array_view_1, TskArrayView array_view_2);
TskEmpty       tsk_array_view_hash(TskArrayView array_view, const TskTraitHasher *trait_hasher, TskAny *hasher);

extern const TskType tsk_array_view_type;

typedef struct TskArrayViewConst TskArrayViewConst;
struct TskArrayViewConst {
  const TskType *element_type;
  const TskAny  *elements;
  TskUSize       length;
  TskISize       stride;
};
TskBoolean           tsk_array_view_const_is_valid(TskArrayViewConst array_view);
TskArrayViewConst tsk_array_view_const_slice(TskArrayViewConst array_view, TskUSize start, TskUSize end, TskISize step);
const TskType    *tsk_array_view_const_element_type(TskArrayViewConst array_view);
TskUSize          tsk_array_view_const_length(TskArrayViewConst array_view);
TskBoolean           tsk_array_view_const_is_empty(TskArrayViewConst array_view);
TskISize          tsk_array_view_const_stride(TskArrayViewConst array_view);
const TskAny     *tsk_array_view_const_get(TskArrayViewConst array_view, TskUSize index);
const TskAny     *tsk_array_view_const_front(TskArrayViewConst array_view);
const TskAny     *tsk_array_view_const_back(TskArrayViewConst array_view);
TskBoolean           tsk_array_view_const_linear_search(TskArrayViewConst array_view, const TskAny *element, TskUSize *index);
TskBoolean           tsk_array_view_const_binary_search(TskArrayViewConst array_view, const TskAny *element, TskUSize *index);
TskUSize          tsk_array_view_const_lower_bound(TskArrayViewConst array_view, const TskAny *element);
TskUSize          tsk_array_view_const_upper_bound(TskArrayViewConst array_view, const TskAny *element);
TskEmpty          tsk_array_view_const_equal_range(TskArrayViewConst array_view, const TskAny *element, TskUSize *start, TskUSize *end);
TskBoolean           tsk_array_view_const_is_sorted(TskArrayViewConst array_view);
TskUSize          tsk_array_view_const_sorted_until(TskArrayViewConst array_view);
TskBoolean           tsk_array_view_const_is_partitioned(TskArrayViewConst array_view, TskBoolean (*predicate)(const TskAny *element));
TskUSize          tsk_array_view_const_partition_point(TskArrayViewConst array_view, TskBoolean (*predicate)(const TskAny *element));
TskOrdering       tsk_array_view_const_compare(TskArrayViewConst array_view_1, TskArrayViewConst array_view_2);
TskBoolean           tsk_array_view_const_equals(TskArrayViewConst array_view_1, TskArrayViewConst array_view_2);
TskEmpty          tsk_array_view_const_hash(TskArrayViewConst array_view, const TskTraitHasher *trait_hasher, TskAny *hasher);

extern const TskType tsk_array_view_const_type;

TskArrayView      tsk_array_to_view(TskArray *array);
TskArrayViewConst tsk_array_to_view_const(const TskArray *array);

TskArrayViewConst tsk_array_view_to_const(TskArrayView array_view); */

#ifdef __cplusplus
}
#endif

#endif // TSK_ARRAY_H_INCLUDED
