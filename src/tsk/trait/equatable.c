#include <tsk/trait/equatable.h>

#include <tsk/trait/complete.h>

#include <assert.h>
#include <string.h>

TskBoolean tsk_trait_equatable_equals(const TskType *equatable_type, const TskAny *equatable_1, const TskAny *equatable_2) {
	assert(tsk_type_is_valid(equatable_type));
	assert(equatable_1 != TSK_NULL);
	assert(equatable_2 != TSK_NULL);

	const TskTraitEquatable *equatable_trait = tsk_type_trait(equatable_type, TSK_TRAIT_ID_EQUATABLE);
	assert(equatable_trait != TSK_NULL);

	if (equatable_trait->equals != TSK_NULL) {
		return equatable_trait->equals(equatable_type, equatable_1, equatable_2);
	}

	return memcmp(equatable_1, equatable_2, tsk_trait_complete_size(equatable_type)) == 0;
}
