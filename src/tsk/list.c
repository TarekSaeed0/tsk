#include <tsk/list.h>

#include <tsk/default_hasher.h>
#include <tsk/reference.h>
#include <tsk/trait/builder.h>
#include <tsk/trait/clonable.h>
#include <tsk/trait/comparable.h>
#include <tsk/trait/complete.h>
#include <tsk/trait/droppable.h>
#include <tsk/trait/equatable.h>
#include <tsk/trait/hashable.h>
#include <tsk/trait/hasher.h>

#include <assert.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct TskListType TskListType;
struct TskListType {
	TskType                list_type;
	TskCharacter           list_type_name[40];
	TskTypeTraitTable      list_type_trait_table;
	TskTypeTraitTableEntry list_type_trait_table_entries[16];
	const TskType         *element_type;
};

struct TskListNode {
	TskListNode *next;
	TskListNode *previous;
	alignas(max_align_t) TskU8 element[];
};

TskBoolean tsk_list_is_valid(const TskType *list_type, const TskList *list) {
	assert(tsk_list_type_is_valid(list_type));

	if (list == TSK_NULL || list->head == TSK_NULL ^ list->tail == TSK_NULL) {
		return TSK_FALSE;
	}

	TskUSize     length   = 0;
	TskListNode *current  = list->head;
	TskListNode *previous = TSK_NULL;
	while (current != TSK_NULL) {
		if (current->previous != previous) {
			return TSK_FALSE;
		}

		length++;

		previous = current;
		current  = current->next;
	}

	return list->tail == previous && list->length == length;
}
TskList tsk_list_new(const TskType *list_type) {
	assert(tsk_list_type_is_valid(list_type));

	TskList list = {
		.head   = TSK_NULL,
		.tail   = TSK_NULL,
		.length = 0,
	};

	assert(tsk_list_is_valid(list_type, &list));

	return list;
}
TskEmpty tsk_list_drop(const TskType *list_type, TskList *list) {
	assert(tsk_list_type_is_valid(list_type));
	assert(tsk_list_is_valid(list_type, list));

	tsk_list_clear(list_type, list);
}
TskBoolean tsk_list_clone(const TskType *list_type, const TskList *list_1, TskList *list_2) {
	assert(tsk_list_type_is_valid(list_type));
	assert(tsk_list_is_valid(list_type, list_1));
	assert(list_2 != TSK_NULL);

	if (tsk_list_is_empty(list_type, list_1)) {
		*list_2 = tsk_list_new(list_type);
		return TSK_TRUE;
	}

	TskListNode *head = TSK_NULL;
	TskListNode *tail = TSK_NULL;

	for (TskListNode *current = list_1->head; current != TSK_NULL; current = current->next) {
		TskListNode *node = malloc(sizeof(TskListNode) + tsk_trait_complete_size(tsk_list_element_type(list_type)));
		if (node == TSK_NULL) {
			goto cleanup;
		}

		node->next     = TSK_NULL;
		node->previous = TSK_NULL;

		if (!tsk_trait_clonable_clone(
		        tsk_list_element_type(list_type),
		        current->element,
		        node->element
		    )) {
			goto cleanup;
		}

		if (head == TSK_NULL) {
			head = node;
			tail = node;
		} else {
			tail->next     = node;
			node->previous = tail;
			tail           = node;
		}
	}

	list_2->head = head;
	list_2->tail = tail;

	return TSK_TRUE;
cleanup:
	while (head != TSK_NULL) {
		TskListNode *next = head->next;

		tsk_trait_droppable_drop(
		    tsk_list_element_type(list_type),
		    head->element
		);
		free(head);

		head = next;
	}
	return TSK_FALSE;
}
const TskType *tsk_list_element_type(const TskType *list_type) {
	assert(tsk_list_type_is_valid(list_type));

	return ((const TskListType *)list_type)->element_type;
}
TskUSize tsk_list_length(const TskType *list_type, const TskList *list) {
	assert(tsk_list_type_is_valid(list_type));
	assert(tsk_list_is_valid(list_type, list));

	return list->length;
}
TskBoolean tsk_list_is_empty(const TskType *list_type, const TskList *list) {
	assert(tsk_list_type_is_valid(list_type));
	assert(tsk_list_is_valid(list_type, list));

	return list->length == 0;
}
TskAny *tsk_list_front(const TskType *list_type, TskList *list) {
	assert(tsk_list_type_is_valid(list_type));
	assert(tsk_list_is_valid(list_type, list));

	if (tsk_list_is_empty(list_type, list)) {
		return TSK_NULL;
	}

	return list->head->element;
}
const TskAny *tsk_list_front_const(const TskType *list_type, const TskList *list) {
	assert(tsk_list_type_is_valid(list_type));
	assert(tsk_list_is_valid(list_type, list));

	if (tsk_list_is_empty(list_type, list)) {
		return TSK_NULL;
	}

	return list->head->element;
}
TskAny *tsk_list_back(const TskType *list_type, TskList *list) {
	assert(tsk_list_type_is_valid(list_type));
	assert(tsk_list_is_valid(list_type, list));

	if (tsk_list_is_empty(list_type, list)) {
		return TSK_NULL;
	}

	return list->tail->element;
}
const TskAny *tsk_list_back_const(const TskType *list_type, const TskList *list) {
	assert(tsk_list_type_is_valid(list_type));
	assert(tsk_list_is_valid(list_type, list));

	if (tsk_list_is_empty(list_type, list)) {
		return TSK_NULL;
	}

	return list->tail->element;
}
TskEmpty tsk_list_clear(const TskType *list_type, TskList *list) {
	assert(tsk_list_type_is_valid(list_type));
	assert(tsk_list_is_valid(list_type, list));

	for (TskListNode *current = list->head; current != TSK_NULL;) {
		TskListNode *next = current->next;

		tsk_trait_droppable_drop(
		    tsk_list_element_type(list_type),
		    current->element
		);
		free(current);

		current = next;
	}

	list->head   = TSK_NULL;
	list->tail   = TSK_NULL;
	list->length = 0;
}
TskBoolean tsk_list_push_front(const TskType *list_type, TskList *list, TskAny *element) {
	assert(tsk_list_type_is_valid(list_type));
	assert(tsk_list_is_valid(list_type, list));
	assert(element != TSK_NULL);

	TskListNode *node = malloc(sizeof(TskListNode) + tsk_trait_complete_size(tsk_list_element_type(list_type)));
	if (node == TSK_NULL) {
		return TSK_NULL;
	}

	node->next     = TSK_NULL;
	node->previous = TSK_NULL;

	memcpy(
	    node->element,
	    element,
	    tsk_trait_complete_size(tsk_list_element_type(list_type))
	);

	if (tsk_list_is_empty(list_type, list)) {
		list->head = node;
		list->tail = node;
	} else {
		node->next           = list->head;
		list->head->previous = node;
		list->head           = node;
	}

	list->length++;

	return TSK_TRUE;
}
TskBoolean tsk_list_push_back(const TskType *list_type, TskList *list, TskAny *element) {
	assert(tsk_list_type_is_valid(list_type));
	assert(tsk_list_is_valid(list_type, list));
	assert(element != TSK_NULL);

	TskListNode *node = malloc(sizeof(TskListNode) + tsk_trait_complete_size(tsk_list_element_type(list_type)));
	if (node == TSK_NULL) {
		return TSK_NULL;
	}

	node->next     = TSK_NULL;
	node->previous = TSK_NULL;

	memcpy(
	    node->element,
	    element,
	    tsk_trait_complete_size(tsk_list_element_type(list_type))
	);

	if (tsk_list_is_empty(list_type, list)) {
		list->head = node;
		list->tail = node;
	} else {
		node->previous   = list->tail;
		list->tail->next = node;
		list->tail       = node;
	}

	list->length++;

	return TSK_TRUE;
}
TskBoolean tsk_list_pop_front(const TskType *list_type, TskList *list, TskAny *element) {
	assert(tsk_list_type_is_valid(list_type));
	assert(tsk_list_is_valid(list_type, list));

	if (tsk_list_is_empty(list_type, list)) {
		return TSK_FALSE;
	}

	TskListNode *node = list->head;

	list->head        = node->next;
	if (list->head != TSK_NULL) {
		list->head->previous = TSK_NULL;
	} else {
		list->tail = TSK_NULL;
	}

	if (element != TSK_NULL) {
		memcpy(
		    element,
		    node->element,
		    tsk_trait_complete_size(tsk_list_element_type(list_type))
		);
	} else {
		tsk_trait_droppable_drop(
		    tsk_list_element_type(list_type),
		    node->element
		);
	}
	free(node);

	list->length--;

	return TSK_TRUE;
}
TskBoolean tsk_list_pop_back(const TskType *list_type, TskList *list, TskAny *element) {
	assert(tsk_list_type_is_valid(list_type));
	assert(tsk_list_is_valid(list_type, list));

	if (tsk_list_is_empty(list_type, list)) {
		return TSK_FALSE;
	}

	TskListNode *node = list->tail;

	list->tail        = node->previous;
	if (list->tail != TSK_NULL) {
		list->tail->next = TSK_NULL;
	} else {
		list->head = TSK_NULL;
	}

	if (element != TSK_NULL) {
		memcpy(
		    element,
		    node->element,
		    tsk_trait_complete_size(tsk_list_element_type(list_type))
		);
	} else {
		tsk_trait_droppable_drop(
		    tsk_list_element_type(list_type),
		    node->element
		);
	}
	free(node);

	list->length--;

	return TSK_TRUE;
}
TskOrdering tsk_list_compare(const TskType *list_type, const TskList *list_1, const TskList *list_2) {
	assert(tsk_list_type_is_valid(list_type));
	assert(tsk_list_is_valid(list_type, list_1));
	assert(tsk_list_is_valid(list_type, list_2));
	assert(tsk_type_has_trait(tsk_list_element_type(list_type), TSK_TRAIT_ID_COMPARABLE));

	TskListNode *current_1 = list_1->head;
	TskListNode *current_2 = list_2->head;

	for (; current_1 != TSK_NULL && current_2 != TSK_NULL; current_1 = current_1->next, current_2 = current_2->next) {
		TskOrdering ordering = tsk_trait_comparable_compare(
		    tsk_list_element_type(list_type),
		    current_1->element,
		    current_2->element
		);
		if (ordering != TSK_ORDERING_EQUAL) {
			return ordering;
		}
	}

	return (current_1 != TSK_NULL) - (current_2 != TSK_NULL);
}
TskBoolean tsk_list_equals(const TskType *list_type, const TskList *list_1, const TskList *list_2) {
	assert(tsk_list_type_is_valid(list_type));
	assert(tsk_list_is_valid(list_type, list_1));
	assert(tsk_list_is_valid(list_type, list_2));
	assert(tsk_type_has_trait(tsk_list_element_type(list_type), TSK_TRAIT_ID_EQUATABLE));

	TskListNode *current_1 = list_1->head;
	TskListNode *current_2 = list_2->head;

	for (; current_1 != TSK_NULL && current_2 != TSK_NULL; current_1 = current_1->next, current_2 = current_2->next) {
		if (!tsk_trait_equatable_equals(
		        tsk_list_element_type(list_type),
		        current_1->element,
		        current_2->element
		    )) {
			return TSK_FALSE;
		}
	}

	return current_1 == TSK_NULL && current_2 == TSK_NULL;
}
TskEmpty tsk_list_hash(const TskType *list_type, const TskList *list, const TskType *hasher_type, TskAny *hasher) {
	assert(tsk_list_type_is_valid(list_type));
	assert(tsk_list_is_valid(list_type, list));
	assert(tsk_type_has_trait(tsk_list_element_type(list_type), TSK_TRAIT_ID_COMPARABLE));
	assert(tsk_type_is_valid(hasher_type));
	assert(tsk_type_has_trait(hasher_type, TSK_TRAIT_ID_HASHER));
	assert(hasher != TSK_NULL);

	TskUSize length = tsk_list_length(list_type, list);
	tsk_trait_hasher_combine(
	    hasher_type,
	    hasher,
	    (const TskU8 *)&length,
	    sizeof(length)
	);

	for (const TskListNode *current = list->head; current != TSK_NULL; current = current->next) {
		tsk_trait_hashable_hash(
		    tsk_list_element_type(list_type),
		    current->element,
		    hasher_type,
		    hasher
		);
	}
}

TskEmpty tsk_list_type_trait_droppable_drop(const TskType *droppable_type, TskAny *droppable) {
	tsk_list_drop(droppable_type, droppable);
}
TskBoolean tsk_list_type_trait_clonable_clone(const TskType *clonable_type, const TskAny *clonable_1, TskAny *clonable_2) {
	return tsk_list_clone(clonable_type, clonable_1, clonable_2);
}
TskOrdering tsk_list_type_trait_comparable_compare(const TskType *comparable_type, const TskAny *comparable_1, const TskAny *comparable_2) {
	return tsk_list_compare(comparable_type, comparable_1, comparable_2);
}
TskBoolean tsk_list_type_trait_equatable_equals(const TskType *equatable_type, const TskAny *equatable_1, const TskAny *equatable_2) {
	return tsk_list_equals(equatable_type, equatable_1, equatable_2);
}
TskEmpty tsk_list_type_trait_hashable_hash(const TskType *hashable_type, const TskAny *hashable, const TskType *hasher_type, TskAny *hasher) {
	tsk_list_hash(hashable_type, hashable, hasher_type, hasher);
}

const TskTraitComplete tsk_list_type_trait_complete = {
	.size      = sizeof(TskList),
	.alignment = alignof(TskList),
};
const TskTraitDroppable tsk_list_type_trait_droppable = {
	.drop = tsk_list_type_trait_droppable_drop,
};
const TskTraitClonable tsk_list_type_trait_clonable = {
	.clone = tsk_list_type_trait_clonable_clone,
};
const TskTraitComparable tsk_list_type_trait_comparable = {
	.compare = tsk_list_type_trait_comparable_compare,
};
const TskTraitEquatable tsk_list_type_trait_equatable = {
	.equals = tsk_list_type_trait_equatable_equals,
};
const TskTraitHashable tsk_list_type_trait_hashable = {
	.hash = tsk_list_type_trait_hashable_hash,
};

#define TSK_LIST_TYPES_CAPACITY ((TskUSize)1 << 7)

TskListType tsk_list_types[TSK_LIST_TYPES_CAPACITY];

TskBoolean tsk_list_type_is_valid(const TskType *list_type) {
	return tsk_type_is_valid(list_type) &&
	       &tsk_list_types[0] <= (const TskListType *)list_type && (const TskListType *)list_type < &tsk_list_types[TSK_LIST_TYPES_CAPACITY];
}
const TskType *tsk_list_type(const TskType *element_type) {
	assert(tsk_type_is_valid(element_type));
	assert(tsk_type_has_trait(element_type, TSK_TRAIT_ID_COMPLETE));

	const TskType             *hasher_type = tsk_trait_builder_built_type(tsk_default_hasher_builder_type);
	alignas(max_align_t) TskU8 hasher[tsk_trait_complete_size(hasher_type)];
	tsk_trait_builder_build(tsk_default_hasher_builder_type, tsk_default_hasher_builder, hasher);

	tsk_trait_hasher_combine(hasher_type, hasher, (const TskU8 *)&element_type, sizeof(element_type)); // NOLINT(bugprone-sizeof-expression)
	TskU64 hash = tsk_trait_hasher_finalize(hasher_type, hasher);

	tsk_trait_droppable_drop(hasher_type, hasher);

	TskUSize starting_index = hash & (TSK_LIST_TYPES_CAPACITY - 1);
	TskUSize index          = starting_index;
	while (tsk_list_types[index].element_type != TSK_NULL) {
		if (tsk_list_types[index].element_type == element_type) {
			return &tsk_list_types[index].list_type;
		}
		index = (index + 1) & (TSK_LIST_TYPES_CAPACITY - 1);
		if (index == starting_index) {
			return TSK_NULL;
		}
	}

	tsk_list_types[index].list_type.trait_table                                                                                             = &tsk_list_types[index].list_type_trait_table;
	tsk_list_types[index].list_type_trait_table.entries                                                                                     = tsk_list_types[index].list_type_trait_table_entries;
	tsk_list_types[index].list_type_trait_table.capacity                                                                                    = sizeof(tsk_list_types[index].list_type_trait_table_entries) / sizeof(tsk_list_types[index].list_type_trait_table_entries[0]);

	tsk_list_types[index].list_type_trait_table.entries[TSK_TRAIT_ID_COMPLETE & (tsk_list_types[index].list_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
		.trait_id   = TSK_TRAIT_ID_COMPLETE,
		.trait_data = &tsk_list_type_trait_complete,
	};
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_DROPPABLE)) {
		tsk_list_types[index].list_type_trait_table.entries[TSK_TRAIT_ID_DROPPABLE & (tsk_list_types[index].list_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_DROPPABLE,
			.trait_data = &tsk_list_type_trait_droppable,
		};
	}
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_CLONABLE)) {
		tsk_list_types[index].list_type_trait_table.entries[TSK_TRAIT_ID_CLONABLE & (tsk_list_types[index].list_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_CLONABLE,
			.trait_data = &tsk_list_type_trait_clonable,
		};
	}
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_COMPARABLE)) {
		tsk_list_types[index].list_type_trait_table.entries[TSK_TRAIT_ID_COMPARABLE & (tsk_list_types[index].list_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_COMPARABLE,
			.trait_data = &tsk_list_type_trait_comparable,
		};
	}
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_EQUATABLE)) {
		tsk_list_types[index].list_type_trait_table.entries[TSK_TRAIT_ID_EQUATABLE & (tsk_list_types[index].list_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_EQUATABLE,
			.trait_data = &tsk_list_type_trait_equatable,
		};
	}
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_HASHABLE)) {
		tsk_list_types[index].list_type_trait_table.entries[TSK_TRAIT_ID_HASHABLE & (tsk_list_types[index].list_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_HASHABLE,
			.trait_data = &tsk_list_type_trait_hashable,
		};
	}

	tsk_list_types[index].element_type = element_type;

	(void)snprintf(
	    tsk_list_types[index].list_type_name,
	    sizeof(tsk_list_types[index].list_type_name),
	    "TskList<%s>",
	    tsk_type_name(element_type)
	);
	tsk_list_types[index].list_type.name = tsk_list_types[index].list_type_name;

	const TskType *list_type             = &tsk_list_types[index].list_type;

	assert(tsk_list_type_is_valid(list_type));

	return list_type;
}

typedef struct TskListIteratorType TskListIteratorType;
struct TskListIteratorType {
	TskType        list_iterator_type;
	TskCharacter   list_iterator_type_name[40];
	const TskType *list_type;
};

TskBoolean tsk_list_iterator_is_valid(const TskType *iterator_type, const TskListIterator *iterator) {
	assert(tsk_list_iterator_type_is_valid(iterator_type));

	return iterator != TSK_NULL && tsk_list_is_valid(tsk_list_iterator_list_type(iterator_type), iterator->list);
}
const TskType *tsk_list_iterator_list_type(const TskType *iterator_type) {
	assert(tsk_list_iterator_type_is_valid(iterator_type));

	return ((const TskListIteratorType *)iterator_type)->list_type;
}
const TskType *tsk_list_iterator_item_type(const TskType *iterator_type) {
	assert(tsk_list_iterator_type_is_valid(iterator_type));

	return tsk_reference_type(tsk_list_element_type(tsk_list_iterator_list_type(iterator_type)));
}
TskBoolean tsk_list_iterator_next(const TskType *iterator_type, TskListIterator *iterator, TskAny *item) {
	assert(tsk_list_iterator_is_valid(iterator_type, iterator));
	assert(item != TSK_NULL);

	if (iterator->current == TSK_NULL) {
		return TSK_FALSE;
	}

	memcpy(
	    item,
	    &iterator->current->element,
	    tsk_trait_complete_size(tsk_list_iterator_item_type(iterator_type))
	);

	iterator->current = iterator->current->next;

	return TSK_TRUE;
}

// clang-format off
TSK_TYPE(tsk_list_iterator_type_, TskListIterator,
	TSK_TYPE_TRAIT(tsk_list_iterator_type_, TSK_TRAIT_ID_COMPLETE, &(TskTraitComplete){
		.size      = sizeof(TskListIterator),
		.alignment = alignof(TskListIterator),
	}),
	TSK_TYPE_TRAIT(tsk_list_iterator_type_, TSK_TRAIT_ID_DROPPABLE, &(TskTraitDroppable){
		.drop = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_list_iterator_type_, TSK_TRAIT_ID_CLONABLE, &(TskTraitClonable){
		.clone = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_list_iterator_type_, TSK_TRAIT_ID_EQUATABLE, &(TskTraitEquatable){
		.equals = TSK_NULL,
	}),
);
// clang-format on

#define TSK_LIST_ITERATOR_TYPES_CAPACITY ((TskUSize)1 << 7)

TskListIteratorType tsk_list_iterator_types[TSK_LIST_ITERATOR_TYPES_CAPACITY];

TskBoolean tsk_list_iterator_type_is_valid(const TskType *list_iterator_type) {
	return tsk_type_is_valid(list_iterator_type) &&
	       &tsk_list_iterator_types[0] <= (const TskListIteratorType *)list_iterator_type && (const TskListIteratorType *)list_iterator_type < &tsk_list_iterator_types[TSK_LIST_ITERATOR_TYPES_CAPACITY];
}
const TskType *tsk_list_iterator_type(const TskType *list_type) {
	assert(tsk_list_type_is_valid(list_type));

	const TskType             *hasher_type = tsk_trait_builder_built_type(tsk_default_hasher_builder_type);
	alignas(max_align_t) TskU8 hasher[tsk_trait_complete_size(hasher_type)];
	tsk_trait_builder_build(tsk_default_hasher_builder_type, tsk_default_hasher_builder, hasher);

	tsk_trait_hasher_combine(hasher_type, hasher, (const TskU8 *)&list_type, sizeof(list_type)); // NOLINT(bugprone-sizeof-expression)
	TskU64 hash = tsk_trait_hasher_finalize(hasher_type, hasher);

	tsk_trait_droppable_drop(hasher_type, hasher);

	TskUSize starting_index = hash & (TSK_LIST_ITERATOR_TYPES_CAPACITY - 1);
	TskUSize index          = starting_index;
	while (tsk_list_iterator_types[index].list_type != TSK_NULL) {
		if (tsk_list_iterator_types[index].list_type == list_type) {
			return &tsk_list_iterator_types[index].list_iterator_type;
		}
		index = (index + 1) & (TSK_LIST_ITERATOR_TYPES_CAPACITY - 1);
		if (index == starting_index) {
			return TSK_NULL;
		}
	}

	tsk_list_iterator_types[index].list_iterator_type = *tsk_list_iterator_type_;
	tsk_list_iterator_types[index].list_type          = list_type;

	(void)snprintf(
	    tsk_list_iterator_types[index].list_iterator_type_name,
	    sizeof(tsk_list_iterator_types[index].list_iterator_type_name),
	    "TskListIterator<%s>",
	    tsk_type_name(list_type)
	);
	tsk_list_iterator_types[index].list_iterator_type.name = tsk_list_iterator_types[index].list_iterator_type_name;

	const TskType *list_iterator_type                      = &tsk_list_iterator_types[index].list_iterator_type;

	assert(tsk_list_iterator_type_is_valid(list_iterator_type));

	return list_iterator_type;
}

TskListIterator tsk_list_iterator(const TskType *list_type, TskList *list) {
	assert(tsk_list_type_is_valid(list_type));
	assert(tsk_list_is_valid(list_type, list));

	TskListIterator list_iterator = {
		.list    = list,
		.current = list->head,
	};

	assert(tsk_list_iterator_is_valid(tsk_list_iterator_type(list_type), &list_iterator));

	return list_iterator;
}
