#ifndef TSK_TRAIT_DROPPABLE_H_INCLUDED
#define TSK_TRAIT_DROPPABLE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <tsk/type.h>

#define TSK_TRAIT_ID_DROPPABLE ((TskTraitID)1)
typedef struct TskTraitDroppable TskTraitDroppable;
struct TskTraitDroppable {
	TskEmpty (*drop)(const TskType *droppable_type, TskAny *droppable);
};
TskEmpty tsk_trait_droppable_drop(const TskType *droppable_type, TskAny *droppable);

#ifdef __cplusplus
}
#endif

#endif // TSK_TRAIT_DROPPABLE_H_INCLUDED
