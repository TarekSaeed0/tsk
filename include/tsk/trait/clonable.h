#ifndef TSK_TRAIT_CLONABLE_H_INCLUDED
#define TSK_TRAIT_CLONABLE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <tsk/type.h>

#define TSK_TRAIT_ID_CLONABLE ((TskTraitID)2)
typedef struct TskTraitClonable TskTraitClonable;
struct TskTraitClonable {
	TskBoolean (*clone)(const TskType *clonable_type, const TskAny *clonable_1, TskAny *clonable_2);
};
TskBoolean tsk_trait_clonable_clone(const TskType *clonable_type, const TskAny *clonable_1, TskAny *clonable_2);

#ifdef __cplusplus
}
#endif

#endif // TSK_TRAIT_CLONABLE_H_INCLUDED
