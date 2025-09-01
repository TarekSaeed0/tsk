#include <tsk/trait/comparable.h>

#include <assert.h>

TskOrdering tsk_trait_comparable_compare(const TskType *comparable_type, const TskAny *comparable_1, const TskAny *comparable_2) {
	assert(tsk_type_is_valid(comparable_type));
	assert(comparable_1 != TSK_NULL);
	assert(comparable_2 != TSK_NULL);

	const TskTraitComparable *comparable_trait = tsk_type_trait(comparable_type, TSK_TRAIT_ID_COMPARABLE);
	assert(comparable_trait != TSK_NULL);
	assert(comparable_trait->compare != TSK_NULL);

	return comparable_trait->compare(comparable_type, comparable_1, comparable_2);
}
