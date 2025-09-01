#ifndef TSK_REFERENCE_H_INCLUDED
#define TSK_REFERENCE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <tsk/trait/comparable.h>
#include <tsk/type.h>

typedef TskAny TskReference;

const TskType *tsk_reference_referenced_type(const TskType *reference_type);
TskAny        *tsk_reference_dereference(const TskType *reference_type, const TskReference *reference);

TskBoolean     tsk_reference_type_is_valid(const TskType *reference_type);
const TskType *tsk_reference_type(const TskType *referenced_type);

typedef TskAny TskReferenceConst;

const TskType *tsk_reference_const_referenced_type(const TskType *reference_type);
const TskAny  *tsk_reference_const_dereference(const TskType *reference_type, const TskReferenceConst *reference);

TskBoolean     tsk_reference_const_type_is_valid(const TskType *reference_type);
const TskType *tsk_reference_const_type(const TskType *referenced_type);

#ifdef __cplusplus
}
#endif

#endif // TSK_REFERENCE_H_INCLUDED
