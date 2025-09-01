#include <tsk/trait/clonable.h>

#include <tsk/trait/complete.h>

#include <assert.h>
#include <string.h>

TskBoolean tsk_trait_clonable_clone(const TskType *clonable_type, const TskAny *clonable_1, TskAny *clonable_2) {
	assert(tsk_type_is_valid(clonable_type));
	assert(clonable_1 != TSK_NULL);
	assert(clonable_2 != TSK_NULL);

	const TskTraitClonable *clonable_trait = tsk_type_trait(clonable_type, TSK_TRAIT_ID_CLONABLE);
	assert(clonable_trait != TSK_NULL);

	if (clonable_trait->clone != TSK_NULL) {
		return clonable_trait->clone(clonable_type, clonable_1, clonable_2);
	}

	memcpy(clonable_2, clonable_1, tsk_trait_complete_size(clonable_type));

	return TSK_TRUE;
}
