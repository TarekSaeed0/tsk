#include <tsk/trait/hasher.h>

#include <assert.h>

TskEmpty tsk_trait_hasher_combine(const TskType *hasher_type, TskAny *hasher, const TskU8 *bytes, TskUSize length) {
	assert(tsk_type_is_valid(hasher_type));
	assert(hasher != TSK_NULL);
	assert(bytes != TSK_NULL || length == 0);

	const TskTraitHasher *hasher_trait = tsk_type_trait(hasher_type, TSK_TRAIT_ID_HASHER);
	assert(hasher_trait != TSK_NULL);
	assert(hasher_trait->combine != TSK_NULL);

	hasher_trait->combine(hasher_type, hasher, bytes, length);
}
TskU64 tsk_trait_hasher_finalize(const TskType *hasher_type, const TskAny *hasher) {
	assert(tsk_type_is_valid(hasher_type));
	assert(hasher != TSK_NULL);

	const TskTraitHasher *hasher_trait = tsk_type_trait(hasher_type, TSK_TRAIT_ID_HASHER);
	assert(hasher_trait != TSK_NULL);
	assert(hasher_trait->finalize != TSK_NULL);

	return hasher_trait->finalize(hasher_type, hasher);
}
