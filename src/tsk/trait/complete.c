#include <tsk/trait/complete.h>

#include <assert.h>

TskUSize tsk_trait_complete_size(const TskType *complete_type) {
	assert(tsk_type_is_valid(complete_type));

	const TskTraitComplete *complete_trait = tsk_type_trait(complete_type, TSK_TRAIT_ID_COMPLETE);
	assert(complete_trait != TSK_NULL);

	return complete_trait->size;
}
TskUSize tsk_trait_complete_alignment(const TskType *complete_type) {
	assert(tsk_type_is_valid(complete_type));

	const TskTraitComplete *complete_trait = tsk_type_trait(complete_type, TSK_TRAIT_ID_COMPLETE);
	assert(complete_trait != TSK_NULL);

	TskUSize alignment = complete_trait->alignment;

	assert(alignment > 0 && (alignment & (alignment - 1)) == 0);

	return alignment;
}
