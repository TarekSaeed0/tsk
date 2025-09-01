#include <tsk/trait/droppable.h>

#include <assert.h>

TskEmpty tsk_trait_droppable_drop(const TskType *droppable_type, TskAny *droppable) {
	assert(tsk_type_is_valid(droppable_type));
	assert(droppable != TSK_NULL);

	const TskTraitDroppable *droppable_trait = tsk_type_trait(droppable_type, TSK_TRAIT_ID_DROPPABLE);
	assert(droppable_trait != TSK_NULL);

	if (droppable_trait->drop != TSK_NULL) {
		droppable_trait->drop(droppable_type, droppable);
	}
}
