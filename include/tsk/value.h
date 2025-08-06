#ifndef TSK_VALUE_H_INCLUDED
#define TSK_VALUE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <tsk/type.h>

typedef struct TskValue TskValue;
struct TskValue {
	alignas(max_align_t) union {
		TskU8   stack_allocated[(sizeof(TskAny *) + sizeof(const TskType *) + (-(sizeof(TskAny *) + sizeof(const TskType *)) & (alignof(max_align_t) - 1))) - sizeof(const TskType *)];
		TskAny *heap_allocated;
	} data;
	const TskType *type;
};
TskBool        tsk_value_is_valid(const TskValue *value);
TskBool        tsk_value_new(TskValue *value, const TskType *type);
TskEmpty       tsk_value_drop(TskValue *value);
const TskType *tsk_value_type(const TskValue *value);
TskAny        *tsk_value_data(TskValue *value);
const TskAny  *tsk_value_data_const(const TskValue *value);

#ifdef __cplusplus
}
#endif

#endif // TSK_VALUE_H_INCLUDED
