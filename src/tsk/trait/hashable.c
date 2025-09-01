#include <tsk/trait/hashable.h>

#include <tsk/trait/complete.h>
#include <tsk/trait/hasher.h>

#include <assert.h>

TskEmpty tsk_trait_hashable_hash(const TskType *hashable_type, const TskAny *hashable, const TskType *hasher_type, TskAny *hasher) {
	assert(tsk_type_is_valid(hashable_type));
	assert(hashable != TSK_NULL);
	assert(tsk_type_is_valid(hasher_type));
	assert(hasher != TSK_NULL);

	const TskTraitHashable *hashable_trait = tsk_type_trait(hashable_type, TSK_TRAIT_ID_HASHABLE);
	assert(hashable_trait != TSK_NULL);

	if (hashable_trait->hash != TSK_NULL) {
		hashable_trait->hash(hashable_type, hashable, hasher_type, hasher);
		return;
	}

	tsk_trait_hasher_combine(hasher_type, hasher, (const TskU8 *)hashable, tsk_trait_complete_size(hashable_type));
}
