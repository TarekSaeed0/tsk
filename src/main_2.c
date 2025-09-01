#include <tsk/type.h>

#include <tsk/list.h>
#include <tsk/reference.h>
#include <tsk/trait/clonable.h>
#include <tsk/trait/complete.h>
#include <tsk/trait/droppable.h>
#include <tsk/trait/iterator.h>
#include <tsk/tuple.h>

#include <assert.h>
#include <stdio.h>

typedef struct FibonacciIterator FibonacciIterator;
struct FibonacciIterator {
	TskU64 a;
	TskU64 b;
};
FibonacciIterator fibonacci_iterator_new(TskEmpty) {
	FibonacciIterator fibonacci_iterator = {
		.a = 0,
		.b = 1,
	};

	return fibonacci_iterator;
}
TskBoolean fibonacci_iterator_next(FibonacciIterator *fibonacci_iterator, TskU64 *item) {

	*item                 = fibonacci_iterator->a;
	TskU64 c              = fibonacci_iterator->a + fibonacci_iterator->b;
	fibonacci_iterator->a = fibonacci_iterator->b;
	fibonacci_iterator->b = c;

	return TSK_TRUE;
}

const TskType *fibonacci_iterator_type_trait_iterator_item_type(const TskType *iterator_type) {
	(TskEmpty) iterator_type;
	return tsk_u64_type;
}
TskBoolean fibonacci_iterator_type_trait_iterator_next(const TskType *iterator_type, TskAny *iterator, TskAny *item) {
	(TskEmpty) iterator_type;
	return fibonacci_iterator_next(iterator, item);
}

// clang-format off
TSK_TYPE(fibonacci_iterator_type, FibonacciIterator,
	TSK_TYPE_TRAIT(fibonacci_iterator_type, TSK_TRAIT_ID_COMPLETE, &(TskTraitComplete){
		.size      = sizeof(FibonacciIterator),
		.alignment = alignof(FibonacciIterator),
	}),
	TSK_TYPE_TRAIT(fibonacci_iterator_type, TSK_TRAIT_ID_DROPPABLE, &(TskTraitDroppable){
		.drop = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(fibonacci_iterator_type, TSK_TRAIT_ID_CLONABLE, &(TskTraitClonable){
		.clone = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(fibonacci_iterator_type, TSK_TRAIT_ID_ITERATOR, &(TskTraitIterator){
		.item_type = fibonacci_iterator_type_trait_iterator_item_type,
		.next          = fibonacci_iterator_type_trait_iterator_next,
	}),
);
// clang-format on

void inspect_type(const TskType *type) {
	assert(tsk_type_is_valid(type));

	printf("type information:\n");
	printf("\tname: %s\n", type->name);
	printf("\ttraits:\n");
	for (TskUSize i = 0; i < type->trait_table->capacity; i++) {
		if (type->trait_table->entries[i].trait_data != TSK_NULL) {
			printf("\t\ttrait_id: %zu, trait_data: %p\n", type->trait_table->entries[i].trait_id, type->trait_table->entries[i].trait_data);
		}
	}
}

TskU64 sum_u64_iterator(const TskType *iterator_type, TskAny *iterator, TskU64 n) {
	assert(tsk_type_is_valid(iterator_type));
	assert(tsk_type_has_trait(iterator_type, TSK_TRAIT_ID_ITERATOR));
	assert(iterator != TSK_NULL);

	TskU64 sum = 0;
	for (TskU64 i = 0; i < n; i++) {
		TskU64 item = 0;
		if (!tsk_trait_iterator_next(iterator_type, iterator, &item)) {
			break;
		}
		sum += item;
	}

	return sum;
}

TskEmpty tsk_list_extend_from_iterator(const TskType *list_type, TskList *list, const TskType *iterator_type, TskAny *iterator, TskU64 limit) {
	assert(tsk_list_type_is_valid(list_type));
	assert(tsk_list_is_valid(list_type, list));
	assert(tsk_type_is_valid(iterator_type));
	assert(tsk_type_has_trait(iterator_type, TSK_TRAIT_ID_ITERATOR));
	assert(iterator != TSK_NULL);

	alignas(max_align_t) TskU8 item[tsk_trait_complete_size(tsk_trait_iterator_item_type(iterator_type))];
	for (TskU64 i = 0; i < limit && tsk_trait_iterator_next(iterator_type, iterator, &item); i++) {
		tsk_list_push_back(list_type, list, &item);
	}
}

int main(void) {
	const TskType *list_type             = tsk_list_type(tsk_u64_type);
	TskList        list                  = tsk_list_new(list_type);

	FibonacciIterator fibonacci_iterator = fibonacci_iterator_new();
	tsk_list_extend_from_iterator(list_type, &list, fibonacci_iterator_type, &fibonacci_iterator, 10);

	const TskType  *iterator_type = tsk_list_iterator_type(tsk_u64_type);
	TskListIterator iterator      = tsk_list_iterator(iterator_type, &list);

	{
		for (TskU64 *item = TSK_NULL; tsk_list_iterator_next(iterator_type, &iterator, (TskAny *)&item);) {
			printf("%llu ", (unsigned long long)*item);
		}
		printf("\n");
	}

	tsk_list_drop(list_type, &list);
}
