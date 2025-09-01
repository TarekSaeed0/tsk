#ifndef TSK_TRAIT_EQUATABLE_H_INCLUDED
#define TSK_TRAIT_EQUATABLE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <tsk/type.h>

#define TSK_TRAIT_ID_EQUATABLE ((TskTraitID)4)
typedef struct TskTraitEquatable TskTraitEquatable;
struct TskTraitEquatable {
	TskBoolean (*equals)(const TskType *equatable_type, const TskAny *equatable_1, const TskAny *equatable_2);
};
TskBoolean tsk_trait_equatable_equals(const TskType *equatable_type, const TskAny *equatable_1, const TskAny *equatable_2);

#ifdef __cplusplus
}
#endif

#endif // TSK_TRAIT_EQUATABLE_H_INCLUDED
