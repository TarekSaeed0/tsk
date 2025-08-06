#include <tsk/list.h>

#include <assert.h>
#include <stdalign.h>
#include <stdlib.h>
#include <string.h>

struct TskListNode {
	TskListNode *next;
	TskListNode *previous;
	alignas(max_align_t) TskU8 element[];
};

TskBool tsk_list_is_valid(const TskList *list) {
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
TskList tsk_list_new(const TskType *element_type) {
	assert(tsk_type_is_valid(element_type));

	TskList list = {
		.element_type = element_type,
		.head         = TSK_NULL,
		.tail         = TSK_NULL,
		.length       = 0,
	};

	assert(tsk_list_is_valid(&list));

	return list;
}
TskEmpty tsk_list_drop(TskList *list) {
	assert(tsk_list_is_valid(list));

	tsk_list_clear(list);
}
TskBool tsk_list_clone(const TskList *list_1, TskList *list_2) {
	assert(tsk_list_is_valid(list_1));
	assert(list_2 != TSK_NULL);

	if (tsk_list_is_empty(list_1)) {
		*list_2 = tsk_list_new(list_1->element_type);
		return TSK_TRUE;
	}

	TskListNode *head = TSK_NULL;
	TskListNode *tail = TSK_NULL;

	for (TskListNode *current = list_1->head; current != TSK_NULL; current = current->next) {
		TskListNode *node = malloc(sizeof(TskListNode) + tsk_trait_complete_size(tsk_type_trait(list_1->element_type, TSK_TRAIT_COMPLETE)));
		if (node == TSK_NULL) {
			goto cleanup;
		}

		node->next     = TSK_NULL;
		node->previous = TSK_NULL;

		if (!tsk_trait_clonable_clone(
		        tsk_type_trait(list_1->element_type, TSK_TRAIT_CLONABLE),
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

	list_2->element_type = list_1->element_type;
	list_2->head         = head;
	list_2->tail         = tail;

	return TSK_TRUE;
cleanup:
	while (head != TSK_NULL) {
		TskListNode *next = head->next;

		tsk_trait_droppable_drop(
		    tsk_type_trait(list_1->element_type, TSK_TRAIT_DROPPABLE),
		    head->element
		);
		free(head);

		head = next;
	}
	return TSK_FALSE;
}
TskUSize tsk_list_length(const TskList *list) {
	assert(tsk_list_is_valid(list));

	return list->length;
}
TskBool tsk_list_is_empty(const TskList *list) {
	assert(tsk_list_is_valid(list));

	return list->length == 0;
}
TskAny *tsk_list_front(TskList *list) {
	assert(tsk_list_is_valid(list));

	if (tsk_list_is_empty(list)) {
		return TSK_NULL;
	}

	return list->head->element;
}
const TskAny *tsk_list_front_const(const TskList *list) {
	assert(tsk_list_is_valid(list));

	if (tsk_list_is_empty(list)) {
		return TSK_NULL;
	}

	return list->head->element;
}
TskAny *tsk_list_back(TskList *list) {
	assert(tsk_list_is_valid(list));

	if (tsk_list_is_empty(list)) {
		return TSK_NULL;
	}

	return list->tail->element;
}
const TskAny *tsk_list_back_const(const TskList *list) {
	assert(tsk_list_is_valid(list));

	if (tsk_list_is_empty(list)) {
		return TSK_NULL;
	}

	return list->tail->element;
}
TskEmpty tsk_list_clear(TskList *list) {
	assert(tsk_list_is_valid(list));

	for (TskListNode *current = list->head; current != TSK_NULL;) {
		TskListNode *next = current->next;

		tsk_trait_droppable_drop(
		    tsk_type_trait(list->element_type, TSK_TRAIT_DROPPABLE),
		    current->element
		);
		free(current);

		current = next;
	}

	list->head   = TSK_NULL;
	list->tail   = TSK_NULL;
	list->length = 0;
}
TskBool tsk_list_push_front(TskList *list, TskAny *element) {
	assert(tsk_list_is_valid(list));
	assert(element != TSK_NULL);

	TskListNode *node = malloc(sizeof(TskListNode) + tsk_trait_complete_size(tsk_type_trait(list->element_type, TSK_TRAIT_COMPLETE)));
	if (node == TSK_NULL) {
		return TSK_NULL;
	}

	node->next     = TSK_NULL;
	node->previous = TSK_NULL;

	memcpy(
	    node->element,
	    element,
	    tsk_trait_complete_size(tsk_type_trait(list->element_type, TSK_TRAIT_COMPLETE))
	);

	if (tsk_list_is_empty(list)) {
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
TskBool tsk_list_push_back(TskList *list, TskAny *element) {
	assert(tsk_list_is_valid(list));
	assert(element != TSK_NULL);

	TskListNode *node = malloc(sizeof(TskListNode) + tsk_trait_complete_size(tsk_type_trait(list->element_type, TSK_TRAIT_COMPLETE)));
	if (node == TSK_NULL) {
		return TSK_NULL;
	}

	node->next     = TSK_NULL;
	node->previous = TSK_NULL;

	memcpy(
	    node->element,
	    element,
	    tsk_trait_complete_size(tsk_type_trait(list->element_type, TSK_TRAIT_COMPLETE))
	);

	if (tsk_list_is_empty(list)) {
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
TskBool tsk_list_pop_front(TskList *list, TskAny *element) {
	assert(tsk_list_is_valid(list));

	if (tsk_list_is_empty(list)) {
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
		    tsk_trait_complete_size(tsk_type_trait(list->element_type, TSK_TRAIT_COMPLETE))
		);
	} else {
		tsk_trait_droppable_drop(
		    tsk_type_trait(list->element_type, TSK_TRAIT_DROPPABLE),
		    node->element
		);
	}
	free(node);

	list->length--;

	return TSK_TRUE;
}
TskBool tsk_list_pop_back(TskList *list, TskAny *element) {
	assert(tsk_list_is_valid(list));

	if (tsk_list_is_empty(list)) {
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
		    tsk_trait_complete_size(tsk_type_trait(list->element_type, TSK_TRAIT_COMPLETE))
		);
	} else {
		tsk_trait_droppable_drop(
		    tsk_type_trait(list->element_type, TSK_TRAIT_DROPPABLE),
		    node->element
		);
	}
	free(node);

	list->length--;

	return TSK_TRUE;
}
TskOrdering tsk_list_compare(const TskList *list_1, const TskList *list_2) {
	assert(tsk_list_is_valid(list_1));
	assert(tsk_list_is_valid(list_2));
	assert(list_1->element_type == list_2->element_type);
	assert(tsk_type_has_trait(list_1->element_type, TSK_TRAIT_COMPARABLE));

	TskListNode *current_1 = list_1->head;
	TskListNode *current_2 = list_2->head;

	for (; current_1 != TSK_NULL && current_2 != TSK_NULL; current_1 = current_1->next, current_2 = current_2->next) {
		TskOrdering ordering = tsk_trait_comparable_compare(
		    tsk_type_trait(list_1->element_type, TSK_TRAIT_COMPARABLE),
		    current_1->element,
		    current_2->element
		);
		if (ordering != 0) {
			return ordering;
		}
	}

	return (current_1 != TSK_NULL) - (current_2 != TSK_NULL);
}
TskBool tsk_list_equals(const TskList *list_1, const TskList *list_2) {
	assert(tsk_list_is_valid(list_1));
	assert(tsk_list_is_valid(list_2));
	assert(list_1->element_type == list_2->element_type);
	assert(tsk_type_has_trait(list_1->element_type, TSK_TRAIT_EQUATABLE));

	TskListNode *current_1 = list_1->head;
	TskListNode *current_2 = list_2->head;

	for (; current_1 != TSK_NULL && current_2 != TSK_NULL; current_1 = current_1->next, current_2 = current_2->next) {
		if (!tsk_trait_equatable_equals(
		        tsk_type_trait(list_1->element_type, TSK_TRAIT_EQUATABLE),
		        current_1->element,
		        current_2->element
		    )) {
			return TSK_FALSE;
		}
	}

	return current_1 == TSK_NULL && current_2 == TSK_NULL;
}
TskEmpty tsk_list_hash(const TskList *list, const TskTraitHasher *trait_hasher, TskAny *hasher) {
	assert(tsk_list_is_valid(list));
	assert(tsk_type_has_trait(list->element_type, TSK_TRAIT_HASHABLE));

	tsk_usize_hash(tsk_list_length(list), trait_hasher, hasher);
	for (const TskListNode *current = list->head; current != TSK_NULL; current = current->next) {
		tsk_trait_hashable_hash(
		    tsk_type_trait(list->element_type, TSK_TRAIT_HASHABLE),
		    current->element,
		    trait_hasher,
		    hasher
		);
	}
}

static const TskTraitComplete tsk_list_trait_complete = {
	.size      = sizeof(TskList),
	.alignment = alignof(TskList),
};

static TskEmpty tsk_list_trait_droppable_drop(TskAny *list) {
	tsk_list_drop(list);
}
static const TskTraitDroppable tsk_list_trait_droppable = {
	.drop = tsk_list_trait_droppable_drop,
};

static TskBool tsk_list_trait_clonable_clone(const TskAny *list_1, TskAny *list_2) {
	return tsk_list_clone(list_1, list_2);
}
static const TskTraitClonable tsk_list_trait_clonable = {
	.trait_complete = &tsk_list_trait_complete,
	.clone          = tsk_list_trait_clonable_clone,
};

static TskOrdering tsk_list_trait_comparable_compare(const TskAny *list_1, const TskAny *list_2) {
	return tsk_list_compare(list_1, list_2);
}
static const TskTraitComparable tsk_list_trait_comparable = {
	.compare = tsk_list_trait_comparable_compare,
};

static TskBool tsk_list_type_equatable_equals(const TskAny *list_1, const TskAny *list_2) {
	return tsk_list_equals(list_1, list_2);
}
static const TskTraitEquatable tsk_list_trait_equatable = {
	.equals = tsk_list_type_equatable_equals,
};

static TskEmpty tsk_list_trait_hashable_hash(const TskAny *list, const TskTraitHasher *trait_hasher, TskAny *hasher) {
	tsk_list_hash(list, trait_hasher, hasher);
}
static const TskTraitHashable tsk_list_trait_hashable = {
	.hash = tsk_list_trait_hashable_hash,
};

const TskType tsk_list_type = {
	.name   = "TskList",
	.traits = {
	    [TSK_TRAIT_COMPLETE]   = &tsk_list_trait_complete,
	    [TSK_TRAIT_DROPPABLE]  = &tsk_list_trait_droppable,
	    [TSK_TRAIT_CLONABLE]   = &tsk_list_trait_clonable,
	    [TSK_TRAIT_COMPARABLE] = &tsk_list_trait_comparable,
	    [TSK_TRAIT_EQUATABLE]  = &tsk_list_trait_equatable,
	    [TSK_TRAIT_HASHABLE]   = &tsk_list_trait_hashable,
	},
};
