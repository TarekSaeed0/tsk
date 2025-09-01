#ifndef TSK_TRAIT_COMPLETE_H_INCLUDED
#define TSK_TRAIT_COMPLETE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <tsk/type.h>

#define TSK_TRAIT_ID_COMPLETE ((TskTraitID)0)
typedef struct TskTraitComplete TskTraitComplete;
struct TskTraitComplete {
	TskUSize size;
	TskUSize alignment;
};
TskUSize tsk_trait_complete_size(const TskType *complete_type);
TskUSize tsk_trait_complete_alignment(const TskType *complete_type);

#ifdef __cplusplus
}
#endif

#endif // TSK_TRAIT_COMPLETE_H_INCLUDED
