#include <tsk/trait/iterator.h>

#include <tsk/trait/complete.h>

#include <assert.h>

const TskType *tsk_trait_iterator_item_type(const TskType *iterator_type) {
	assert(tsk_type_is_valid(iterator_type));

	const TskTraitIterator *iterator_trait = tsk_type_trait(iterator_type, TSK_TRAIT_ID_ITERATOR);
	assert(iterator_trait != TSK_NULL);
	assert(iterator_trait->item_type != TSK_NULL);

	const TskType *built_type = iterator_trait->item_type(iterator_type);
	assert(tsk_type_is_valid(built_type));
	assert(tsk_type_has_trait(built_type, TSK_TRAIT_ID_COMPLETE));

	return built_type;
}
TskBoolean tsk_trait_iterator_next(const TskType *iterator_type, TskAny *iterator, TskAny *item) {
	assert(tsk_type_is_valid(iterator_type));
	assert(iterator != TSK_NULL);
	assert(item != TSK_NULL);

	const TskTraitIterator *iterator_trait = tsk_type_trait(iterator_type, TSK_TRAIT_ID_ITERATOR);
	assert(iterator_trait != TSK_NULL);
	assert(iterator_trait->next != TSK_NULL);

	return iterator_trait->next(iterator_type, iterator, item);
}
