#ifndef TSK_TUPLE_H_INCLUDED
#define TSK_TUPLE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <tsk/trait/comparable.h>
#include <tsk/type.h>

typedef TskAny TskTuple;

TskEmpty       tsk_tuple_drop(const TskType *tuple_type, TskTuple *tuple);
TskBoolean     tsk_tuple_clone(const TskType *tuple_type, const TskTuple *tuple_1, TskTuple *tuple_2);
const TskType *tsk_tuple_element_type(const TskType *tuple_type, TskUSize index);
TskUSize       tsk_tuple_length(const TskType *tuple_type);
TskAny        *tsk_tuple_get(const TskType *tuple_type, TskTuple *tuple, TskUSize index);
const TskAny  *tsk_tuple_get_const(const TskType *tuple_type, const TskTuple *tuple, TskUSize index);
TskOrdering    tsk_tuple_compare(const TskType *tuple_type, const TskTuple *tuple_1, const TskTuple *tuple_2);
TskBoolean     tsk_tuple_equals(const TskType *tuple_type, const TskTuple *tuple_1, const TskTuple *tuple_2);
TskEmpty       tsk_tuple_hash(const TskType *tuple_type, const TskTuple *tuple, const TskType *hasher_type, TskAny *hasher);

TskBoolean     tsk_tuple_type_is_valid(const TskType *tuple_type);
const TskType *tsk_tuple_type(const TskType **element_types, TskUSize length);

#ifdef __cplusplus
}
#endif

#endif // TSK_TUPLE_H_INCLUDED
