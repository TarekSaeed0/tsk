#ifndef TSK_DEQUE_H_INCLUDED
#define TSK_DEQUE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <tsk/array.h>
#include <tsk/trait/comparable.h>
#include <tsk/type.h>

typedef struct TskDeque TskDeque;
struct TskDeque {
	TskAny **segments;
	TskUSize segments_length;
	TskUSize front_index;
	TskUSize back_index;
};
TskBoolean     tsk_deque_is_valid(const TskType *deque_type, const TskDeque *deque);
TskDeque       tsk_deque_new(const TskType *deque_type);
TskEmpty       tsk_deque_drop(const TskType *deque_type, TskDeque *deque);
TskBoolean     tsk_deque_clone(const TskType *deque_type, const TskDeque *deque_1, TskDeque *deque_2);
const TskType *tsk_deque_element_type(const TskType *deque_type);
TskUSize       tsk_deque_length(const TskType *deque_type, const TskDeque *deque);
TskBoolean     tsk_deque_is_empty(const TskType *deque_type, const TskDeque *deque);
TskUSize       tsk_deque_capacity(const TskType *deque_type, const TskDeque *deque);
TskAny        *tsk_deque_get(const TskType *deque_type, TskDeque *deque, TskUSize index);
const TskAny  *tsk_deque_get_const(const TskType *deque_type, const TskDeque *deque, TskUSize index);
TskAny        *tsk_deque_front(const TskType *deque_type, TskDeque *deque);
const TskAny  *tsk_deque_front_const(const TskType *deque_type, const TskDeque *deque);
TskAny        *tsk_deque_back(const TskType *deque_type, TskDeque *deque);
const TskAny  *tsk_deque_back_const(const TskType *deque_type, const TskDeque *deque);
TskEmpty       tsk_deque_clear(const TskType *deque_type, TskDeque *deque);
TskBoolean     tsk_deque_push_front(const TskType *deque_type, TskDeque *deque, TskAny *element);
TskBoolean     tsk_deque_push_back(const TskType *deque_type, TskDeque *deque, TskAny *element);
TskBoolean     tsk_deque_pop_front(const TskType *deque_type, TskDeque *deque, TskAny *element);
TskBoolean     tsk_deque_pop_back(const TskType *deque_type, TskDeque *deque, TskAny *element);
TskOrdering    tsk_deque_compare(const TskType *deque_type, const TskDeque *deque_1, const TskDeque *deque_2);
TskBoolean     tsk_deque_equals(const TskType *deque_type, const TskDeque *deque_1, const TskDeque *deque_2);
TskEmpty       tsk_deque_hash(const TskType *deque_type, const TskDeque *deque, const TskType *hasher_type, TskAny *hasher);

TskBoolean     tsk_deque_type_is_valid(const TskType *deque_type);
const TskType *tsk_deque_type(const TskType *element_type);

#ifdef __cplusplus
}
#endif

#endif // TSK_DEQUE_H_INCLUDED
