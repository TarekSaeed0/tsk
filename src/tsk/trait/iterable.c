#include <tsk/trait/iterable.h>

#include <tsk/trait/complete.h>

#include <assert.h>

const TskType *tsk_trait_iterable_iterator_type(const TskType *iterable_type) {
	assert(tsk_type_is_valid(iterable_type));

	const TskTraitIterable *iterable_trait = tsk_type_trait(iterable_type, TSK_TRAIT_ID_ITERABLE);
	assert(iterable_trait != TSK_NULL);
	assert(iterable_trait->iterator_type != TSK_NULL);

	const TskType *built_type = iterable_trait->iterator_type(iterable_type);
	assert(tsk_type_is_valid(built_type));
	assert(tsk_type_has_trait(built_type, TSK_TRAIT_ID_COMPLETE));

	return built_type;
}
TskEmpty tsk_trait_iterable_iterator(const TskType *iterable_type, TskAny *iterable, TskAny *iterator) {
	assert(tsk_type_is_valid(iterable_type));
	assert(iterable != TSK_NULL);
	assert(iterator != TSK_NULL);

	const TskTraitIterable *iterable_trait = tsk_type_trait(iterable_type, TSK_TRAIT_ID_ITERABLE);
	assert(iterable_trait != TSK_NULL);
	assert(iterable_trait->iterator != TSK_NULL);

	iterable_trait->iterator(iterable_type, iterable, iterator);
}

const TskType *tsk_trait_iterable_const_iterator_type(const TskType *iterable_type) {
	assert(tsk_type_is_valid(iterable_type));

	const TskTraitIterableConst *iterable_trait = tsk_type_trait(iterable_type, TSK_TRAIT_ID_ITERABLE);
	assert(iterable_trait != TSK_NULL);
	assert(iterable_trait->iterator_type != TSK_NULL);

	const TskType *built_type = iterable_trait->iterator_type(iterable_type);
	assert(tsk_type_is_valid(built_type));
	assert(tsk_type_has_trait(built_type, TSK_TRAIT_ID_COMPLETE));

	return built_type;
}
TskEmpty tsk_trait_iterable_const_iterator(const TskType *iterable_type, const TskAny *iterable, TskAny *iterator) {
	assert(tsk_type_is_valid(iterable_type));
	assert(iterable != TSK_NULL);
	assert(iterator != TSK_NULL);

	const TskTraitIterableConst *iterable_trait = tsk_type_trait(iterable_type, TSK_TRAIT_ID_ITERABLE);
	assert(iterable_trait != TSK_NULL);
	assert(iterable_trait->iterator != TSK_NULL);

	iterable_trait->iterator(iterable_type, iterable, iterator);
}
