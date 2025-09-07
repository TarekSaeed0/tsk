#define _GNU_SOURCE
#include <tsk/array.h>

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
#include <tsk/trait/iterable.h>
#include <tsk/trait/iterator.h>

#include <assert.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: make array, array_view and array_view_const implement Iterable

typedef struct TskArrayType TskArrayType;
struct TskArrayType {
	TskType                array_type;
	TskCharacter           array_type_name[40];
	TskTypeTraitTable      array_type_trait_table;
	TskTypeTraitTableEntry array_type_trait_table_entries[16];
	const TskType         *element_type;
};

TskBoolean tsk_array_is_valid(const TskType *array_type, const TskArray *array) {
	assert(tsk_array_type_is_valid(array_type));

	return array != TSK_NULL && (array->elements != TSK_NULL || array->capacity == 0) && array->length <= array->capacity;
}
TskArray tsk_array_new(const TskType *array_type) {
	assert(tsk_array_type_is_valid(array_type));

	TskArray array = {
		.length = 0,
	};

	if (tsk_trait_complete_size(tsk_array_element_type(array_type)) == 0) {
		array.elements = (TskAny *)tsk_trait_complete_alignment(tsk_array_element_type(array_type)); // NOLINT(performance-no-int-to-ptr)
		array.capacity = SIZE_MAX;
	} else {
		array.elements = TSK_NULL;
		array.capacity = 0;
	}

	assert(tsk_array_is_valid(array_type, &array));

	return array;
}
TskEmpty tsk_array_drop(const TskType *array_type, TskArray *array) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));

	tsk_array_clear(array_type, array);

	if (tsk_trait_complete_size(tsk_array_element_type(array_type)) != 0) {
		free(array->elements);
	}

	array->elements = TSK_NULL;
	array->capacity = 0;
}
TskBoolean tsk_array_clone(const TskType *array_type, const TskArray *array_1, TskArray *array_2) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array_1));
	assert(array_2 != TSK_NULL);

	if (tsk_array_is_empty(array_type, array_1)) {
		*array_2 = tsk_array_new(array_type);
		return TSK_TRUE;
	}

	TskAny *elements = TSK_NULL;
	if (tsk_trait_complete_size(tsk_array_element_type(array_type)) == 0) {
		elements = (TskAny *)tsk_trait_complete_alignment(tsk_array_element_type(array_type)); // NOLINT(performance-no-int-to-ptr)
	} else {
		elements = malloc(tsk_array_length(array_type, array_1) * tsk_trait_complete_size(tsk_array_element_type(array_type)));
		if (elements == TSK_NULL) {
			return TSK_FALSE;
		}
	}

	for (TskUSize i = 0; i < tsk_array_length(array_type, array_1); i++) {
		if (!tsk_trait_clonable_clone(
		        tsk_array_element_type(array_type),
		        tsk_array_get_const(array_type, array_1, i),
		        (TskU8 *)elements + (i * tsk_trait_complete_size(tsk_array_element_type(array_type)))
		    )) {
			for (TskUSize j = 0; j < i; j++) {
				tsk_trait_droppable_drop(
				    tsk_array_element_type(array_type),
				    (TskU8 *)elements + (j * tsk_trait_complete_size(tsk_array_element_type(array_type)))
				);
			}
			free(elements);
			return TSK_FALSE;
		}
	}

	array_2->elements = elements;
	array_2->length   = array_1->length;
	array_2->capacity = array_1->capacity;

	return TSK_TRUE;
}
const TskType *tsk_array_element_type(const TskType *array_type) {
	assert(tsk_array_type_is_valid(array_type));

	return ((const TskArrayType *)array_type)->element_type;
}
TskAny *tsk_array_elements(const TskType *array_type, TskArray *array) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));

	return array->elements;
}
const TskAny *tsk_array_elements_const(const TskType *array_type, const TskArray *array) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));

	return array->elements;
}
TskUSize tsk_array_length(const TskType *array_type, const TskArray *array) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));

	return array->length;
}
TskBoolean tsk_array_is_empty(const TskType *array_type, const TskArray *array) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));

	return tsk_array_length(array_type, array) == 0;
}
TskUSize tsk_array_capacity(const TskType *array_type, const TskArray *array) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));

	return array->capacity;
}
TskUSize tsk_array_maximum_capacity(const TskType *array_type) {
	assert(tsk_array_type_is_valid(array_type));

	TskUSize element_size = tsk_trait_complete_size(tsk_array_element_type(array_type));
	return element_size == 0 ? SIZE_MAX : SIZE_MAX / element_size;
}
TskAny *tsk_array_get(const TskType *array_type, TskArray *array, TskUSize index) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));
	assert(index < tsk_array_length(array_type, array));

	return (TskU8 *)array->elements + (index * tsk_trait_complete_size(tsk_array_element_type(array_type)));
}
const TskAny *tsk_array_get_const(const TskType *array_type, const TskArray *array, TskUSize index) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));
	assert(index < tsk_array_length(array_type, array));

	return (const TskU8 *)array->elements + (index * tsk_trait_complete_size(tsk_array_element_type(array_type)));
}
TskAny *tsk_array_front(const TskType *array_type, TskArray *array) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));

	if (tsk_array_is_empty(array_type, array)) {
		return TSK_NULL;
	}

	return tsk_array_get(array_type, array, 0);
}
const TskAny *tsk_array_front_const(const TskType *array_type, const TskArray *array) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));

	if (tsk_array_is_empty(array_type, array)) {
		return TSK_NULL;
	}

	return tsk_array_get_const(array_type, array, 0);
}
TskAny *tsk_array_back(const TskType *array_type, TskArray *array) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));

	if (tsk_array_is_empty(array_type, array)) {
		return TSK_NULL;
	}

	return tsk_array_get(array_type, array, tsk_array_length(array_type, array) - 1);
}
const TskAny *tsk_array_back_const(const TskType *array_type, const TskArray *array) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));

	if (tsk_array_is_empty(array_type, array)) {
		return TSK_NULL;
	}

	return tsk_array_get_const(array_type, array, tsk_array_length(array_type, array) - 1);
}
TskEmpty tsk_array_clear(const TskType *array_type, TskArray *array) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));
	assert(tsk_type_has_trait(tsk_array_element_type(array_type), TSK_TRAIT_ID_DROPPABLE));

	for (TskUSize i = 0; i < tsk_array_length(array_type, array); i++) {
		tsk_trait_droppable_drop(
		    tsk_array_element_type(array_type),
		    tsk_array_get(array_type, array, i)
		);
	}

	array->length = 0;
}
TskBoolean tsk_array_reserve(const TskType *array_type, TskArray *array, TskUSize capacity) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));

	if (capacity <= tsk_array_capacity(array_type, array)) {
		return TSK_TRUE;
	}

	if (capacity > tsk_array_maximum_capacity(array_type)) {
		return TSK_FALSE;
	}

	TskAny *elements = realloc(array->elements, capacity * tsk_trait_complete_size(tsk_array_element_type(array_type)));
	if (elements == TSK_NULL) {
		return TSK_FALSE;
	}

	array->elements = elements;
	array->capacity = capacity;

	return TSK_TRUE;
}
TskBoolean tsk_array_reserve_additional(const TskType *array_type, TskArray *array, TskUSize additional) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));

	TskUSize maximum_capacity = tsk_array_maximum_capacity(array_type);
	if (additional > maximum_capacity - tsk_array_length(array_type, array)) {
		return TSK_FALSE;
	}

	if (tsk_array_capacity(array_type, array) >= tsk_array_length(array_type, array) + additional) {
		return TSK_TRUE;
	}

	TskUSize capacity = tsk_array_capacity(array_type, array);
	do {
		if (capacity == 0) {
			capacity = 1;
		} else if (capacity > maximum_capacity / 2) {
			capacity = maximum_capacity;
			break;
		} else {
			capacity *= 2;
		}
	} while (capacity < tsk_array_length(array_type, array) + additional);

	return tsk_array_reserve(array_type, array, capacity);
}
TskBoolean tsk_array_insert(const TskType *array_type, TskArray *array, TskUSize index, TskAny *elements, TskUSize count) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));
	assert(index <= tsk_array_length(array_type, array));
	assert(elements != TSK_NULL || count == 0);

	if (!tsk_array_reserve_additional(array_type, array, count)) {
		return TSK_FALSE;
	}

	memmove(
	    (TskU8 *)array->elements + ((index + count) * tsk_trait_complete_size(tsk_array_element_type(array_type))),
	    (const TskU8 *)array->elements + (index * tsk_trait_complete_size(tsk_array_element_type(array_type))),
	    (tsk_array_length(array_type, array) - index) * tsk_trait_complete_size(tsk_array_element_type(array_type))
	);
	memcpy(
	    (TskU8 *)array->elements + (index * tsk_trait_complete_size(tsk_array_element_type(array_type))),
	    elements,
	    count * tsk_trait_complete_size(tsk_array_element_type(array_type))
	);

	array->length += count;

	return TSK_TRUE;
}
TskEmpty tsk_array_remove(const TskType *array_type, TskArray *array, TskUSize index, TskAny *elements, TskUSize count) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));
	assert(index < tsk_array_length(array_type, array));
	assert(count < tsk_array_length(array_type, array) - index);

	if (elements != TSK_NULL) {
		memcpy(
		    elements,
		    tsk_array_get_const(array_type, array, index),
		    count * tsk_trait_complete_size(tsk_array_element_type(array_type))
		);
	} else {
		assert(tsk_type_has_trait(tsk_array_element_type(array_type), TSK_TRAIT_ID_DROPPABLE));
		for (TskUSize i = index; i < index + count; i++) {
			tsk_trait_droppable_drop(
			    tsk_array_element_type(array_type),
			    tsk_array_get(array_type, array, i)
			);
		}
	}

	memmove(
	    tsk_array_get(array_type, array, index),
	    tsk_array_get_const(array_type, array, index + count),
	    (tsk_array_length(array_type, array) - index - count) * tsk_trait_complete_size(tsk_array_element_type(array_type))
	);

	array->length -= count;
}
TskBoolean tsk_array_push_front(const TskType *array_type, TskArray *array, TskAny *element) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));
	assert(element != TSK_NULL);

	return tsk_array_insert(array_type, array, 0, element, 1);
}
TskBoolean tsk_array_push_back(const TskType *array_type, TskArray *array, TskAny *element) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));
	assert(element != TSK_NULL);

	return tsk_array_insert(array_type, array, tsk_array_length(array_type, array), element, 1);
}
TskBoolean tsk_array_pop_front(const TskType *array_type, TskArray *array, TskAny *element) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));

	if (tsk_array_is_empty(array_type, array)) {
		return TSK_FALSE;
	}

	tsk_array_remove(array_type, array, 0, element, 1);

	return TSK_TRUE;
}
TskBoolean tsk_array_pop_back(const TskType *array_type, TskArray *array, TskAny *element) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));

	if (tsk_array_is_empty(array_type, array)) {
		return TSK_FALSE;
	}

	tsk_array_remove(array_type, array, tsk_array_length(array_type, array) - 1, element, 1);

	return TSK_TRUE;
}
TskOrdering tsk_array_compare(const TskType *array_type, const TskArray *array_1, const TskArray *array_2) {
	return tsk_array_view_const_compare(
	    tsk_array_view_type(tsk_array_element_type(array_type)),
	    tsk_array_view_const(array_type, array_1),
	    tsk_array_view_const(array_type, array_2)
	);
}
TskBoolean tsk_array_equals(const TskType *array_type, const TskArray *array_1, const TskArray *array_2) {
	return tsk_array_view_const_equals(
	    tsk_array_view_type(tsk_array_element_type(array_type)),
	    tsk_array_view_const(array_type, array_1),
	    tsk_array_view_const(array_type, array_2)
	);
}
TskEmpty tsk_array_hash(const TskType *array_type, const TskArray *array, const TskType *hasher_type, TskAny *hasher) {
	tsk_array_view_const_hash(
	    tsk_array_view_type(tsk_array_element_type(array_type)),
	    tsk_array_view_const(array_type, array),
	    hasher_type,
	    hasher
	);
}

TskEmpty tsk_array_type_trait_droppable_drop(const TskType *droppable_type, TskAny *droppable) {
	tsk_array_drop(droppable_type, droppable);
}
TskBoolean tsk_array_type_trait_clonable_clone(const TskType *clonable_type, const TskAny *clonable_1, TskAny *clonable_2) {
	return tsk_array_clone(clonable_type, clonable_1, clonable_2);
}
TskOrdering tsk_array_type_trait_comparable_compare(const TskType *comparable_type, const TskAny *comparable_1, const TskAny *comparable_2) {
	return tsk_array_compare(comparable_type, comparable_1, comparable_2);
}
TskBoolean tsk_array_type_trait_equatable_equals(const TskType *equatable_type, const TskAny *equatable_1, const TskAny *equatable_2) {
	return tsk_array_equals(equatable_type, equatable_1, equatable_2);
}
TskEmpty tsk_array_type_trait_hashable_hash(const TskType *hashable_type, const TskAny *hashable, const TskType *hasher_type, TskAny *hasher) {
	tsk_array_hash(hashable_type, hashable, hasher_type, hasher);
}
const TskType *tsk_array_type_trait_iterable_iterator_type(const TskType *iterable_type) {
	return tsk_array_iterator_type(iterable_type);
}
TskEmpty tsk_array_type_trait_iterable_iterator(const TskType *iterable_type, TskAny *iterable, TskAny *iterator) {
	*(TskArrayIterator *)iterator = tsk_array_iterator(iterable_type, iterable);
}
const TskType *tsk_array_type_trait_iterable_const_iterator_type(const TskType *iterable_type) {
	return tsk_array_iterator_const_type(iterable_type);
}
TskEmpty tsk_array_type_trait_iterable_const_iterator(const TskType *iterable_type, const TskAny *iterable, TskAny *iterator) {
	*(TskArrayIteratorConst *)iterator = tsk_array_iterator_const(iterable_type, iterable);
}

const TskTraitComplete tsk_array_type_trait_complete = {
	.size      = sizeof(TskArray),
	.alignment = alignof(TskArray),
};
const TskTraitDroppable tsk_array_type_trait_droppable = {
	.drop = tsk_array_type_trait_droppable_drop,
};
const TskTraitClonable tsk_array_type_trait_clonable = {
	.clone = tsk_array_type_trait_clonable_clone,
};
const TskTraitComparable tsk_array_type_trait_comparable = {
	.compare = tsk_array_type_trait_comparable_compare,
};
const TskTraitEquatable tsk_array_type_trait_equatable = {
	.equals = tsk_array_type_trait_equatable_equals,
};
const TskTraitHashable tsk_array_type_trait_hashable = {
	.hash = tsk_array_type_trait_hashable_hash,
};
const TskTraitIterable tsk_array_type_trait_iterable = {
	.iterator_type = tsk_array_type_trait_iterable_iterator_type,
	.iterator      = tsk_array_type_trait_iterable_iterator,
};
const TskTraitIterableConst tsk_array_type_trait_iterable_const = {
	.iterator_type = tsk_array_type_trait_iterable_const_iterator_type,
	.iterator      = tsk_array_type_trait_iterable_const_iterator,
};

#define TSK_ARRAY_TYPES_CAPACITY ((TskUSize)1 << 7)

TskArrayType tsk_array_types[TSK_ARRAY_TYPES_CAPACITY];

TskBoolean tsk_array_type_is_valid(const TskType *array_type) {
	return tsk_type_is_valid(array_type) &&
	       &tsk_array_types[0] <= (const TskArrayType *)array_type && (const TskArrayType *)array_type < &tsk_array_types[TSK_ARRAY_TYPES_CAPACITY];
}
const TskType *tsk_array_type(const TskType *element_type) {
	assert(tsk_type_is_valid(element_type));
	assert(tsk_type_has_trait(element_type, TSK_TRAIT_ID_COMPLETE));

	const TskType             *hasher_type = tsk_trait_builder_built_type(tsk_default_hasher_builder_type);
	alignas(max_align_t) TskU8 hasher[tsk_trait_complete_size(hasher_type)];
	tsk_trait_builder_build(tsk_default_hasher_builder_type, tsk_default_hasher_builder, hasher);

	tsk_trait_hasher_combine(hasher_type, hasher, (const TskU8 *)&element_type, sizeof(element_type)); // NOLINT(bugprone-sizeof-expression)
	TskU64 hash = tsk_trait_hasher_finalize(hasher_type, hasher);

	tsk_trait_droppable_drop(hasher_type, hasher);

	TskUSize starting_index = hash & (TSK_ARRAY_TYPES_CAPACITY - 1);
	TskUSize index          = starting_index;
	while (tsk_array_types[index].element_type != TSK_NULL) {
		if (tsk_array_types[index].element_type == element_type) {
			return &tsk_array_types[index].array_type;
		}
		index = (index + 1) & (TSK_ARRAY_TYPES_CAPACITY - 1);
		if (index == starting_index) {
			return TSK_NULL;
		}
	}

	tsk_array_types[index].array_type.trait_table                                                                                               = &tsk_array_types[index].array_type_trait_table;
	tsk_array_types[index].array_type_trait_table.entries                                                                                       = tsk_array_types[index].array_type_trait_table_entries;
	tsk_array_types[index].array_type_trait_table.capacity                                                                                      = sizeof(tsk_array_types[index].array_type_trait_table_entries) / sizeof(tsk_array_types[index].array_type_trait_table_entries[0]);

	tsk_array_types[index].array_type_trait_table.entries[TSK_TRAIT_ID_COMPLETE & (tsk_array_types[index].array_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
		.trait_id   = TSK_TRAIT_ID_COMPLETE,
		.trait_data = &tsk_array_type_trait_complete,
	};
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_DROPPABLE)) {
		tsk_array_types[index].array_type_trait_table.entries[TSK_TRAIT_ID_DROPPABLE & (tsk_array_types[index].array_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_DROPPABLE,
			.trait_data = &tsk_array_type_trait_droppable,
		};
	}
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_CLONABLE)) {
		tsk_array_types[index].array_type_trait_table.entries[TSK_TRAIT_ID_CLONABLE & (tsk_array_types[index].array_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_CLONABLE,
			.trait_data = &tsk_array_type_trait_clonable,
		};
	}
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_COMPARABLE)) {
		tsk_array_types[index].array_type_trait_table.entries[TSK_TRAIT_ID_COMPARABLE & (tsk_array_types[index].array_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_COMPARABLE,
			.trait_data = &tsk_array_type_trait_comparable,
		};
	}
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_EQUATABLE)) {
		tsk_array_types[index].array_type_trait_table.entries[TSK_TRAIT_ID_EQUATABLE & (tsk_array_types[index].array_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_EQUATABLE,
			.trait_data = &tsk_array_type_trait_equatable,
		};
	}
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_HASHABLE)) {
		tsk_array_types[index].array_type_trait_table.entries[TSK_TRAIT_ID_HASHABLE & (tsk_array_types[index].array_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_HASHABLE,
			.trait_data = &tsk_array_type_trait_hashable,
		};
	}
	tsk_array_types[index].array_type_trait_table.entries[TSK_TRAIT_ID_ITERABLE & (tsk_array_types[index].array_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
		.trait_id   = TSK_TRAIT_ID_ITERABLE,
		.trait_data = &tsk_array_type_trait_iterable,
	};
	tsk_array_types[index].array_type_trait_table.entries[TSK_TRAIT_ID_ITERABLE_CONST & (tsk_array_types[index].array_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
		.trait_id   = TSK_TRAIT_ID_ITERABLE_CONST,
		.trait_data = &tsk_array_type_trait_iterable_const,
	};

	tsk_array_types[index].element_type = element_type;

	(void)snprintf(
	    tsk_array_types[index].array_type_name,
	    sizeof(tsk_array_types[index].array_type_name),
	    "TskArray<%s>",
	    tsk_type_name(element_type)
	);
	tsk_array_types[index].array_type.name = tsk_array_types[index].array_type_name;

	const TskType *array_type              = &tsk_array_types[index].array_type;

	assert(tsk_array_type_is_valid(array_type));

	return array_type;
}

typedef struct TskArrayViewType TskArrayViewType;
struct TskArrayViewType {
	TskType                array_view_type;
	TskCharacter           array_view_type_name[40];
	TskTypeTraitTable      array_view_type_trait_table;
	TskTypeTraitTableEntry array_view_type_trait_table_entries[16];
	const TskType         *element_type;
};

TskBoolean tsk_array_view_is_valid(const TskType *array_view_type, TskArrayView array_view) {
	assert(tsk_array_view_type_is_valid(array_view_type));

	return array_view.elements != TSK_NULL || array_view.length == 0;
}
TskArrayView tsk_array_view_new(const TskType *array_view_type, TskAny *elements, TskUSize length, TskISize stride) {
	assert(tsk_array_view_type_is_valid(array_view_type));
	assert(elements != TSK_NULL || length == 0);
	assert(stride != 0);

	TskArrayView array_view = {
		.elements = elements,
		.length   = length,
		.stride   = stride,
	};

	assert(tsk_array_view_is_valid(array_view_type, array_view));

	return array_view;
}
TskArrayView tsk_array_view_slice(const TskType *array_view_type, TskArrayView array_view, TskUSize start, TskUSize end, TskISize step) {
	assert(tsk_array_view_type_is_valid(array_view_type));
	assert(tsk_array_view_is_valid(array_view_type, array_view));
	assert(step != 0);

	if (step > 0) {
		assert(end <= tsk_array_view_length(array_view_type, array_view));
		assert(start <= end);

		TskArrayView array_view_slice = {
			.element_type = tsk_array_view_element_type(array_view_type),
			.elements     = (TskU8 *)tsk_array_view_elements(array_view_type, array_view) + (start * tsk_trait_complete_size(tsk_array_view_element_type(array_view_type))),
			.length       = (end - start + (TskUSize)step - 1) / (TskUSize)step,
			.stride       = tsk_array_view_stride(array_view_type, array_view) * step,
		};

		assert(tsk_array_view_is_valid(array_view_type, array_view_slice));

		return array_view_slice;
	}

	assert(start <= tsk_array_view_length(array_view_type, array_view));
	assert(end <= start);

	TskArrayView array_view_slice = {
		.elements = (TskU8 *)tsk_array_view_elements(array_view_type, array_view) + (start * tsk_trait_complete_size(tsk_array_view_element_type(array_view_type))) - tsk_trait_complete_size(tsk_array_view_element_type(array_view_type)),
		.length   = (start - end + (TskUSize)-step - 1) / (TskUSize)-step,
		.stride   = tsk_array_view_stride(array_view_type, array_view) * step,
	};

	assert(tsk_array_view_is_valid(array_view_type, array_view_slice));

	return array_view_slice;
}
const TskType *tsk_array_view_element_type(const TskType *array_view_type) {
	assert(tsk_array_view_type_is_valid(array_view_type));

	return ((const TskArrayViewType *)array_view_type)->element_type;
}
TskAny *tsk_array_view_elements(const TskType *array_view_type, TskArrayView array_view) {
	assert(tsk_array_view_type_is_valid(array_view_type));
	assert(tsk_array_view_is_valid(array_view_type, array_view));

	return array_view.elements;
}
TskUSize tsk_array_view_length(const TskType *array_view_type, TskArrayView array_view) {
	assert(tsk_array_view_type_is_valid(array_view_type));
	assert(tsk_array_view_is_valid(array_view_type, array_view));

	return array_view.length;
}
TskBoolean tsk_array_view_is_empty(const TskType *array_view_type, TskArrayView array_view) {
	assert(tsk_array_view_type_is_valid(array_view_type));
	assert(tsk_array_view_is_valid(array_view_type, array_view));

	return tsk_array_view_length(array_view_type, array_view) == 0;
}
TskISize tsk_array_view_stride(const TskType *array_view_type, TskArrayView array_view) {
	assert(tsk_array_view_type_is_valid(array_view_type));
	assert(tsk_array_view_is_valid(array_view_type, array_view));

	return array_view.stride;
}
TskAny *tsk_array_view_get(const TskType *array_view_type, TskArrayView array_view, TskUSize index) {
	assert(tsk_array_view_type_is_valid(array_view_type));
	assert(tsk_array_view_is_valid(array_view_type, array_view));
	assert(index < tsk_array_view_length(array_view_type, array_view));

	if (tsk_array_view_stride(array_view_type, array_view) > 0) {
		return (TskU8 *)tsk_array_view_elements(array_view_type, array_view) + (index * (TskUSize)tsk_array_view_stride(array_view_type, array_view) * tsk_trait_complete_size(tsk_array_view_element_type(array_view_type)));
	}

	return (TskU8 *)tsk_array_view_elements(array_view_type, array_view) - (index * (TskUSize)-tsk_array_view_stride(array_view_type, array_view) * tsk_trait_complete_size(tsk_array_view_element_type(array_view_type)));
}
TskAny *tsk_array_view_front(const TskType *array_view_type, TskArrayView array_view) {
	assert(tsk_array_view_type_is_valid(array_view_type));
	assert(tsk_array_view_is_valid(array_view_type, array_view));

	if (tsk_array_view_is_empty(array_view_type, array_view)) {
		return TSK_NULL;
	}

	return tsk_array_view_get(array_view_type, array_view, 0);
}
TskAny *tsk_array_view_back(const TskType *array_view_type, TskArrayView array_view) {
	assert(tsk_array_view_type_is_valid(array_view_type));
	assert(tsk_array_view_is_valid(array_view_type, array_view));

	if (tsk_array_view_is_empty(array_view_type, array_view)) {
		return TSK_NULL;
	}

	return tsk_array_view_get(array_view_type, array_view, tsk_array_view_length(array_view_type, array_view) - 1);
}
TskEmpty tsk_array_view_swap(const TskType *array_view_type, TskArrayView array_view, TskUSize index_1, TskUSize index_2) {
	assert(tsk_array_view_type_is_valid(array_view_type));
	assert(tsk_array_view_is_valid(array_view_type, array_view));
	assert(index_1 < tsk_array_view_length(array_view_type, array_view));
	assert(index_2 < tsk_array_view_length(array_view_type, array_view));

	if (index_1 == index_2) {
		return;
	}

	TskU8 *elements_1 = (TskU8 *)tsk_array_view_get(array_view_type, array_view, index_1);
	TskU8 *elements_2 = (TskU8 *)tsk_array_view_get(array_view_type, array_view, index_2);
	for (TskUSize i = 0; i < tsk_trait_complete_size(tsk_array_view_element_type(array_view_type)); i++) {
		TskU8 byte    = elements_1[i];
		elements_1[i] = elements_2[i];
		elements_2[i] = byte;
	}
}
#if defined(__GLIBC__) || (defined(__FreeBSD__) && defined(qsort_r))
	#define QSORT_R_LINUX
#elif defined(__APPLE__) || defined(__MACH__) || defined(__DARWIN__) || (defined(__FreeBSD__) && !defined(qsort_r)) || defined(__DragonFly__)
	#define QSORT_R_BSD
#elif defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__) || defined(__MINGW32__) || defined(__MINGW64__)
	#define QSORT_S_WINDOWS
#else
	#error "Platform not supported"
#endif
static int tsk_array_view_sort_compare(
#if defined(QSORT_R_LINUX)
    const void *element_1,
    const void *element_2,
    void       *context
#elif defined(QSORT_R_BSD) || defined(QSORT_S_WINDOWS)
    void       *context,
    const void *element_1,
    const void *element_2
#endif
) {
	const TskType *array_view_type = *(const TskType **)context;
	return tsk_trait_comparable_compare(tsk_array_view_element_type(array_view_type), element_1, element_2);
}
TskEmpty tsk_array_view_sort(const TskType *array_view_type, TskArrayView array_view) {
	assert(tsk_array_view_type_is_valid(array_view_type));
	assert(tsk_array_view_is_valid(array_view_type, array_view));
	assert(tsk_type_has_trait(tsk_array_view_element_type(array_view_type), TSK_TRAIT_ID_COMPARABLE));

	// PERF: find a way to sort, without allocating a buffer and copying the elements

	alignas(max_align_t) TskU8 buffer[1028];

	TskUSize elements_size = tsk_array_view_length(array_view_type, array_view) * tsk_trait_complete_size(tsk_array_view_element_type(array_view_type));
	TskAny  *elements      = TSK_NULL;
	if (elements_size > sizeof(buffer)) {
		elements = malloc(elements_size);
		assert(elements != TSK_NULL);
	} else {
		elements = buffer;
	}

	for (TskUSize i = 0; i < tsk_array_view_length(array_view_type, array_view); i++) {
		memcpy(
		    (TskU8 *)elements + (i * tsk_trait_complete_size(tsk_array_view_element_type(array_view_type))),
		    tsk_array_view_get(array_view_type, array_view, i),
		    tsk_trait_complete_size(tsk_array_view_element_type(array_view_type))
		);
	}

#if defined(QSORT_R_LINUX)
	qsort_r(
	    elements,
	    tsk_array_view_length(array_view_type, array_view),
	    tsk_trait_complete_size(tsk_array_view_element_type(array_view_type)),
	    tsk_array_view_sort_compare,
	    &array_view_type
	);
#elif defined(QSORT_R_BSD)
	qsort_r(
	    elements,
	    tsk_array_view_length(array_view_type, array_view),
	    tsk_trait_complete_size(tsk_array_view_element_type(array_view_type)),
	    &array_view_type,
	    tsk_array_view_sort_compare
	);
#elif defined(QSORT_S_WINDOWS)
	qsort_s(
	    elements,
	    tsk_array_view_length(array_view_type, array_view),
	    tsk_trait_complete_size(tsk_array_view_element_type(array_view_type)),
	    tsk_array_view_sort_compare,
	    &array_view_type
	);
#endif
#undef QSORT_R_LINUX
#undef QSORT_R_BSD
#undef QSORT_S_WINDOWS

	for (TskUSize i = 0; i < tsk_array_view_length(array_view_type, array_view); i++) {
		memcpy(
		    tsk_array_view_get(array_view_type, array_view, i),
		    (TskU8 *)elements + (i * tsk_trait_complete_size(tsk_array_view_element_type(array_view_type))),
		    tsk_trait_complete_size(tsk_array_view_element_type(array_view_type))
		);
	}

	if (elements_size > sizeof(buffer)) {
		free(elements);
	}
}
TskEmpty tsk_array_view_partition(const TskType *array_view_type, TskArrayView array_view, TskBoolean (*predicate)(const TskAny *element)) {
	assert(tsk_array_view_type_is_valid(array_view_type));
	assert(tsk_array_view_is_valid(array_view_type, array_view));
	assert(predicate != TSK_NULL);

	TskUSize i = 0;
	for (; i < tsk_array_view_length(array_view_type, array_view); i++) {
		if (!predicate(tsk_array_view_get(array_view_type, array_view, i))) {
			break;
		}
	}

	for (TskUSize j = i + 1; j < tsk_array_view_length(array_view_type, array_view); j++) {
		if (predicate(tsk_array_view_get(array_view_type, array_view, j))) {
			tsk_array_view_swap(array_view_type, array_view, i, j);
			i++;
		}
	}
}
TskEmpty tsk_array_view_reverse(const TskType *array_view_type, TskArrayView array_view) {
	assert(tsk_array_view_type_is_valid(array_view_type));
	assert(tsk_array_view_is_valid(array_view_type, array_view));

	for (TskUSize i = 0; i < tsk_array_view_length(array_view_type, array_view) / 2; i++) {
		tsk_array_view_swap(array_view_type, array_view, i, tsk_array_view_length(array_view_type, array_view) - 1 - i);
	}
}
TskOrdering tsk_array_view_compare(const TskType *array_view_type, TskArrayView array_view_1, TskArrayView array_view_2) {
	return tsk_array_view_const_compare(
	    tsk_array_view_const_type(tsk_array_view_element_type(array_view_type)),
	    tsk_array_view_as_const(array_view_type, array_view_1),
	    tsk_array_view_as_const(array_view_type, array_view_2)
	);
}
TskBoolean tsk_array_view_equals(const TskType *array_view_type, TskArrayView array_view_1, TskArrayView array_view_2) {
	return tsk_array_view_const_equals(
	    tsk_array_view_const_type(tsk_array_view_element_type(array_view_type)),
	    tsk_array_view_as_const(array_view_type, array_view_1),
	    tsk_array_view_as_const(array_view_type, array_view_2)
	);
}
TskEmpty tsk_array_view_hash(const TskType *array_view_type, TskArrayView array_view, const TskType *hasher_type, TskAny *hasher) {
	tsk_array_view_const_hash(
	    tsk_array_view_const_type(tsk_array_view_element_type(array_view_type)),
	    tsk_array_view_as_const(array_view_type, array_view),
	    hasher_type,
	    hasher
	);
}

typedef struct TskArrayViewConstType TskArrayViewConstType;
struct TskArrayViewConstType {
	TskType                array_view_const_type;
	TskCharacter           array_view_const_type_name[40];
	TskTypeTraitTable      array_view_const_type_trait_table;
	TskTypeTraitTableEntry array_view_const_type_trait_table_entries[16];
	const TskType         *element_type;
};

TskOrdering tsk_array_view_type_trait_comparable_compare(const TskType *comparable_type, const TskAny *comparable_1, const TskAny *comparable_2) {
	return tsk_array_view_compare(comparable_type, *(const TskArrayView *)comparable_1, *(const TskArrayView *)comparable_2);
}
TskBoolean tsk_array_view_type_trait_equatable_equals(const TskType *equatable_type, const TskAny *equatable_1, const TskAny *equatable_2) {
	return tsk_array_view_equals(equatable_type, *(const TskArrayView *)equatable_1, *(const TskArrayView *)equatable_2);
}
TskEmpty tsk_array_view_type_trait_hashable_hash(const TskType *hashable_type, const TskAny *hashable, const TskType *hasher_type, TskAny *hasher) {
	tsk_array_view_hash(hashable_type, *(const TskArrayView *)hashable, hasher_type, hasher);
}

const TskTraitComplete tsk_array_view_type_trait_complete = {
	.size      = sizeof(TskArrayView),
	.alignment = alignof(TskArrayView),
};
const TskTraitDroppable tsk_array_view_type_trait_droppable = {
	.drop = TSK_NULL,
};
const TskTraitClonable tsk_array_view_type_trait_clonable = {
	.clone = TSK_NULL,
};
const TskTraitComparable tsk_array_view_type_trait_comparable = {
	.compare = tsk_array_view_type_trait_comparable_compare,
};
const TskTraitEquatable tsk_array_view_type_trait_equatable = {
	.equals = tsk_array_view_type_trait_equatable_equals,
};
const TskTraitHashable tsk_array_view_type_trait_hashable = {
	.hash = tsk_array_view_type_trait_hashable_hash,
};

#define TSK_ARRAY_VIEW_TYPES_CAPACITY ((TskUSize)1 << 7)

TskArrayViewType tsk_array_view_types[TSK_ARRAY_VIEW_TYPES_CAPACITY];

TskBoolean tsk_array_view_type_is_valid(const TskType *array_view_type) {
	return tsk_type_is_valid(array_view_type) &&
	       &tsk_array_view_types[0] <= (const TskArrayViewType *)array_view_type && (const TskArrayViewType *)array_view_type < &tsk_array_view_types[TSK_ARRAY_VIEW_TYPES_CAPACITY];
}
const TskType *tsk_array_view_type(const TskType *element_type) {
	assert(tsk_type_is_valid(element_type));
	assert(tsk_type_has_trait(element_type, TSK_TRAIT_ID_COMPLETE));

	const TskType             *hasher_type = tsk_trait_builder_built_type(tsk_default_hasher_builder_type);
	alignas(max_align_t) TskU8 hasher[tsk_trait_complete_size(hasher_type)];
	tsk_trait_builder_build(tsk_default_hasher_builder_type, tsk_default_hasher_builder, hasher);

	tsk_trait_hasher_combine(hasher_type, hasher, (const TskU8 *)&element_type, sizeof(element_type)); // NOLINT(bugprone-sizeof-expression)
	TskU64 hash = tsk_trait_hasher_finalize(hasher_type, hasher);

	tsk_trait_droppable_drop(hasher_type, hasher);

	TskUSize starting_index = hash & (TSK_ARRAY_VIEW_TYPES_CAPACITY - 1);
	TskUSize index          = starting_index;
	while (tsk_array_view_types[index].element_type != TSK_NULL) {
		if (tsk_array_view_types[index].element_type == element_type) {
			return &tsk_array_view_types[index].array_view_type;
		}
		index = (index + 1) & (TSK_ARRAY_VIEW_TYPES_CAPACITY - 1);
		if (index == starting_index) {
			return TSK_NULL;
		}
	}

	tsk_array_view_types[index].array_view_type.trait_table                                                                                                         = &tsk_array_view_types[index].array_view_type_trait_table;
	tsk_array_view_types[index].array_view_type_trait_table.entries                                                                                                 = tsk_array_view_types[index].array_view_type_trait_table_entries;
	tsk_array_view_types[index].array_view_type_trait_table.capacity                                                                                                = sizeof(tsk_array_view_types[index].array_view_type_trait_table_entries) / sizeof(tsk_array_view_types[index].array_view_type_trait_table_entries[0]);

	tsk_array_view_types[index].array_view_type_trait_table.entries[TSK_TRAIT_ID_COMPLETE & (tsk_array_view_types[index].array_view_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
		.trait_id   = TSK_TRAIT_ID_COMPLETE,
		.trait_data = &tsk_array_view_type_trait_complete,
	};
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_DROPPABLE)) {
		tsk_array_view_types[index].array_view_type_trait_table.entries[TSK_TRAIT_ID_DROPPABLE & (tsk_array_view_types[index].array_view_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_DROPPABLE,
			.trait_data = &tsk_array_view_type_trait_droppable,
		};
	}
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_CLONABLE)) {
		tsk_array_view_types[index].array_view_type_trait_table.entries[TSK_TRAIT_ID_CLONABLE & (tsk_array_view_types[index].array_view_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_CLONABLE,
			.trait_data = &tsk_array_view_type_trait_clonable,
		};
	}
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_COMPARABLE)) {
		tsk_array_view_types[index].array_view_type_trait_table.entries[TSK_TRAIT_ID_COMPARABLE & (tsk_array_view_types[index].array_view_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_COMPARABLE,
			.trait_data = &tsk_array_view_type_trait_comparable,
		};
	}
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_EQUATABLE)) {
		tsk_array_view_types[index].array_view_type_trait_table.entries[TSK_TRAIT_ID_EQUATABLE & (tsk_array_view_types[index].array_view_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_EQUATABLE,
			.trait_data = &tsk_array_view_type_trait_equatable,
		};
	}
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_HASHABLE)) {
		tsk_array_view_types[index].array_view_type_trait_table.entries[TSK_TRAIT_ID_HASHABLE & (tsk_array_view_types[index].array_view_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_HASHABLE,
			.trait_data = &tsk_array_view_type_trait_hashable,
		};
	}

	tsk_array_view_types[index].element_type = element_type;

	(void)snprintf(
	    tsk_array_view_types[index].array_view_type_name,
	    sizeof(tsk_array_view_types[index].array_view_type_name),
	    "TskArrayView<%s>",
	    tsk_type_name(element_type)
	);
	tsk_array_view_types[index].array_view_type.name = tsk_array_view_types[index].array_view_type_name;

	const TskType *array_view_type                   = &tsk_array_view_types[index].array_view_type;

	assert(tsk_array_view_type_is_valid(array_view_type));

	return array_view_type;
}

TskArrayView tsk_array_view(const TskType *array_type, TskArray *array) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));

	TskArrayView array_view = {
		.elements = tsk_array_elements(array_type, array),
		.length   = tsk_array_length(array_type, array),
		.stride   = 1,
	};

	assert(tsk_array_view_is_valid(tsk_array_view_type(tsk_array_element_type(array_type)), array_view));

	return array_view;
}

TskBoolean tsk_array_view_const_is_valid(const TskType *array_view_type, TskArrayViewConst array_view) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));

	return array_view.elements != TSK_NULL || array_view.length == 0;
}
TskArrayViewConst tsk_array_view_const_new(const TskType *array_view_type, const TskAny *elements, TskUSize length, TskISize stride) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));
	assert(elements != TSK_NULL || length == 0);
	assert(stride != 0);

	TskArrayViewConst array_view = {
		.elements = elements,
		.length   = length,
		.stride   = stride,
	};

	assert(tsk_array_view_const_is_valid(array_view_type, array_view));

	return array_view;
}
TskArrayViewConst tsk_array_view_const_slice(const TskType *array_view_type, TskArrayViewConst array_view, TskUSize start, TskUSize end, TskISize step) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));
	assert(tsk_array_view_const_is_valid(array_view_type, array_view));
	assert(step != 0);

	if (step > 0) {
		assert(end <= tsk_array_view_const_length(array_view_type, array_view));
		assert(start <= end);

		TskArrayViewConst array_view_slice = {
			.elements = (const TskU8 *)tsk_array_view_const_elements(array_view_type, array_view) + (start * tsk_trait_complete_size(tsk_array_view_const_element_type(array_view_type))),
			.length   = (end - start + (TskUSize)step - 1) / (TskUSize)step,
			.stride   = tsk_array_view_const_stride(array_view_type, array_view) * step,
		};

		assert(tsk_array_view_const_is_valid(array_view_type, array_view_slice));

		return array_view_slice;
	}

	assert(start <= tsk_array_view_const_length(array_view_type, array_view));
	assert(end <= start);

	TskArrayViewConst array_view_slice = {
		.elements = (const TskU8 *)tsk_array_view_const_elements(array_view_type, array_view) + (start * tsk_trait_complete_size(tsk_array_view_const_element_type(array_view_type))) - tsk_trait_complete_size(tsk_array_view_const_element_type(array_view_type)),
		.length   = (start - end + (TskUSize)-step - 1) / (TskUSize)-step,
		.stride   = tsk_array_view_const_stride(array_view_type, array_view) * step,
	};

	assert(tsk_array_view_const_is_valid(array_view_type, array_view_slice));

	return array_view_slice;
}
const TskType *tsk_array_view_const_element_type(const TskType *array_view_type) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));

	return ((const TskArrayViewConstType *)array_view_type)->element_type;
}
const TskAny *tsk_array_view_const_elements(const TskType *array_view_type, TskArrayViewConst array_view) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));
	assert(tsk_array_view_const_is_valid(array_view_type, array_view));

	return array_view.elements;
}
TskUSize tsk_array_view_const_length(const TskType *array_view_type, TskArrayViewConst array_view) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));
	assert(tsk_array_view_const_is_valid(array_view_type, array_view));

	return array_view.length;
}
TskBoolean tsk_array_view_const_is_empty(const TskType *array_view_type, TskArrayViewConst array_view) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));
	assert(tsk_array_view_const_is_valid(array_view_type, array_view));

	return tsk_array_view_const_length(array_view_type, array_view) == 0;
}
TskISize tsk_array_view_const_stride(const TskType *array_view_type, TskArrayViewConst array_view) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));
	assert(tsk_array_view_const_is_valid(array_view_type, array_view));

	return array_view.stride;
}
const TskAny *tsk_array_view_const_get(const TskType *array_view_type, TskArrayViewConst array_view, TskUSize index) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));
	assert(tsk_array_view_const_is_valid(array_view_type, array_view));
	assert(index < tsk_array_view_const_length(array_view_type, array_view));

	if (tsk_array_view_const_stride(array_view_type, array_view) > 0) {
		return (const TskU8 *)tsk_array_view_const_elements(array_view_type, array_view) + (index * (TskUSize)tsk_array_view_const_stride(array_view_type, array_view) * tsk_trait_complete_size(tsk_array_view_const_element_type(array_view_type)));
	}

	return (const TskU8 *)tsk_array_view_const_elements(array_view_type, array_view) - (index * (TskUSize)-tsk_array_view_const_stride(array_view_type, array_view) * tsk_trait_complete_size(tsk_array_view_const_element_type(array_view_type)));
}
const TskAny *tsk_array_view_const_front(const TskType *array_view_type, TskArrayViewConst array_view) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));
	assert(tsk_array_view_const_is_valid(array_view_type, array_view));

	if (tsk_array_view_const_is_empty(array_view_type, array_view)) {
		return TSK_NULL;
	}

	return tsk_array_view_const_get(array_view_type, array_view, 0);
}
const TskAny *tsk_array_view_const_back(const TskType *array_view_type, TskArrayViewConst array_view) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));
	assert(tsk_array_view_const_is_valid(array_view_type, array_view));

	if (tsk_array_view_const_is_empty(array_view_type, array_view)) {
		return TSK_NULL;
	}

	return tsk_array_view_const_get(array_view_type, array_view, tsk_array_view_const_length(array_view_type, array_view) - 1);
}
TskBoolean tsk_array_view_const_linear_search(const TskType *array_view_type, TskArrayViewConst array_view, const TskAny *element, TskUSize *index) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));
	assert(tsk_array_view_const_is_valid(array_view_type, array_view));
	assert(element != TSK_NULL);
	assert(tsk_type_has_trait(tsk_array_view_const_element_type(array_view_type), TSK_TRAIT_ID_EQUATABLE));

	for (TskUSize i = 0; i < tsk_array_view_const_length(array_view_type, array_view); i++) {
		if (tsk_trait_equatable_equals(
		        tsk_array_view_const_element_type(array_view_type),
		        tsk_array_view_const_get(array_view_type, array_view, i),
		        element
		    )) {
			if (index != TSK_NULL) {
				*index = i;
			}
			return TSK_TRUE;
		}
	}

	return TSK_FALSE;
}
TskBoolean tsk_array_view_const_binary_search(const TskType *array_view_type, TskArrayViewConst array_view, const TskAny *element, TskUSize *index) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));
	assert(tsk_array_view_const_is_valid(array_view_type, array_view));
	assert(element != TSK_NULL);
	assert(tsk_type_has_trait(tsk_array_view_const_element_type(array_view_type), TSK_TRAIT_ID_COMPARABLE));

	TskUSize left  = 0;
	TskUSize right = tsk_array_view_const_length(array_view_type, array_view) - 1;
	while (left <= right) {
		TskUSize    middle   = left + ((right - left) / 2);
		TskOrdering ordering = tsk_trait_comparable_compare(
		    tsk_array_view_const_element_type(array_view_type),
		    tsk_array_view_const_get(array_view_type, array_view, middle),
		    element
		);
		switch (ordering) {
			case TSK_ORDERING_LESS: {
				left = middle + 1;
			} break;
			case TSK_ORDERING_EQUAL: {
				if (index != TSK_NULL) {
					*index = middle;
				}
				return TSK_TRUE;
			}
			case TSK_ORDERING_GREATER: {
				right = middle - 1;
			} break;
		}
	}

	if (index != TSK_NULL) {
		*index = left;
	}
	return TSK_FALSE;
}
TskUSize tsk_array_view_const_lower_bound(const TskType *array_view_type, TskArrayViewConst array_view, const TskAny *element) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));
	assert(tsk_array_view_const_is_valid(array_view_type, array_view));
	assert(element != TSK_NULL);
	assert(tsk_type_has_trait(tsk_array_view_const_element_type(array_view_type), TSK_TRAIT_ID_COMPARABLE));

	TskUSize left  = 0;
	TskUSize right = tsk_array_view_const_length(array_view_type, array_view) - 1;
	TskUSize index = tsk_array_view_const_length(array_view_type, array_view);
	while (left < right) {
		TskUSize    middle   = left + ((right - left) / 2);
		TskOrdering ordering = tsk_trait_comparable_compare(
		    tsk_array_view_const_element_type(array_view_type),
		    tsk_array_view_const_get(array_view_type, array_view, middle),
		    element
		);
		switch (ordering) {
			case TSK_ORDERING_LESS: {
				left = middle + 1;
			} break;
			case TSK_ORDERING_EQUAL:
			case TSK_ORDERING_GREATER: {
				index = middle;
				right = middle;
			} break;
		}
	}

	return index;
}
TskUSize tsk_array_view_const_upper_bound(const TskType *array_view_type, TskArrayViewConst array_view, const TskAny *element) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));
	assert(tsk_array_view_const_is_valid(array_view_type, array_view));
	assert(element != TSK_NULL);
	assert(tsk_type_has_trait(tsk_array_view_const_element_type(array_view_type), TSK_TRAIT_ID_COMPARABLE));

	TskUSize left  = 0;
	TskUSize right = tsk_array_view_const_length(array_view_type, array_view) - 1;
	TskUSize index = tsk_array_view_const_length(array_view_type, array_view);
	while (left < right) {
		TskUSize    middle   = left + ((right - left) / 2);
		TskOrdering ordering = tsk_trait_comparable_compare(
		    tsk_array_view_const_element_type(array_view_type),
		    tsk_array_view_const_get(array_view_type, array_view, middle),
		    element
		);
		switch (ordering) {
			case TSK_ORDERING_LESS:
			case TSK_ORDERING_EQUAL: {
				left = middle + 1;
			} break;
			case TSK_ORDERING_GREATER: {
				index = middle;
				right = middle;
			} break;
		}
	}

	return index;
}
TskEmpty tsk_array_view_const_equal_range(const TskType *array_view_type, TskArrayViewConst array_view, const TskAny *element, TskUSize *start, TskUSize *end) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));
	assert(tsk_array_view_const_is_valid(array_view_type, array_view));
	assert(element != TSK_NULL);
	assert(tsk_type_has_trait(tsk_array_view_const_element_type(array_view_type), TSK_TRAIT_ID_COMPARABLE));
	assert(start != TSK_NULL);
	assert(end != TSK_NULL);

	*start = tsk_array_view_const_lower_bound(array_view_type, array_view, element);
	*end   = *start + tsk_array_view_const_upper_bound(
                      array_view_type,
                      tsk_array_view_const_slice(
                          array_view_type,
                          array_view,
                          *start,
                          tsk_array_view_const_length(array_view_type, array_view),
                          1
                      ),
                      element
                  );
}
TskBoolean tsk_array_view_const_is_sorted(const TskType *array_view_type, TskArrayViewConst array_view) {
	return tsk_array_view_const_sorted_until(array_view_type, array_view) == tsk_array_view_const_length(array_view_type, array_view);
}
TskUSize tsk_array_view_const_sorted_until(const TskType *array_view_type, TskArrayViewConst array_view) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));
	assert(tsk_array_view_const_is_valid(array_view_type, array_view));
	assert(tsk_type_has_trait(tsk_array_view_const_element_type(array_view_type), TSK_TRAIT_ID_COMPARABLE));

	for (TskUSize i = 1; i < tsk_array_view_const_length(array_view_type, array_view); i++) {
		TskOrdering ordering = tsk_trait_comparable_compare(
		    tsk_array_view_const_element_type(array_view_type),
		    tsk_array_view_const_get(array_view_type, array_view, i - 1),
		    tsk_array_view_const_get(array_view_type, array_view, i)
		);
		if (ordering > 0) {
			return i;
		}
	}

	return tsk_array_view_const_length(array_view_type, array_view);
}
TskBoolean tsk_array_view_const_is_partitioned(const TskType *array_view_type, TskArrayViewConst array_view, TskBoolean (*predicate)(const TskAny *element)) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));
	assert(tsk_array_view_const_is_valid(array_view_type, array_view));

	TskUSize i = 0;
	for (; i < tsk_array_view_const_length(array_view_type, array_view); i++) {
		if (!predicate(tsk_array_view_const_get(array_view_type, array_view, i))) {
			break;
		}
	}

	for (i++; i < tsk_array_view_const_length(array_view_type, array_view); i++) {
		if (predicate(tsk_array_view_const_get(array_view_type, array_view, i))) {
			return TSK_FALSE;
		}
	}

	return TSK_TRUE;
}
TskUSize tsk_array_view_const_partition_point(const TskType *array_view_type, TskArrayViewConst array_view, TskBoolean (*predicate)(const TskAny *element)) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));
	assert(tsk_array_view_const_is_valid(array_view_type, array_view));
	assert(predicate != TSK_NULL);

	TskUSize left  = 0;
	TskUSize right = tsk_array_view_const_length(array_view_type, array_view) - 1;
	TskUSize index = tsk_array_view_const_length(array_view_type, array_view);
	while (left <= right) {
		TskUSize middle = left + ((right - left) / 2);
		if (predicate(tsk_array_view_const_get(array_view_type, array_view, middle))) {
			left = middle + 1;
		} else {
			index = middle;
			right = middle - 1;
		}
	}

	return index;
}
TskOrdering tsk_array_view_const_compare(const TskType *array_view_type, TskArrayViewConst array_view_1, TskArrayViewConst array_view_2) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));
	assert(tsk_array_view_const_is_valid(array_view_type, array_view_1));
	assert(tsk_array_view_const_is_valid(array_view_type, array_view_2));
	assert(tsk_type_has_trait(tsk_array_view_const_element_type(array_view_type), TSK_TRAIT_ID_COMPARABLE));

	for (TskUSize i = 0; i < tsk_array_view_const_length(array_view_type, array_view_1) && i < tsk_array_view_const_length(array_view_type, array_view_2); i++) {
		TskOrdering ordering = tsk_trait_comparable_compare(
		    tsk_array_view_const_element_type(array_view_type),
		    tsk_array_view_const_get(array_view_type, array_view_1, i),
		    tsk_array_view_const_get(array_view_type, array_view_2, i)
		);
		if (ordering != TSK_ORDERING_EQUAL) {
			return ordering;
		}
	}

	return (tsk_array_view_const_length(array_view_type, array_view_1) > tsk_array_view_const_length(array_view_type, array_view_2)) -
	       (tsk_array_view_const_length(array_view_type, array_view_1) < tsk_array_view_const_length(array_view_type, array_view_2));
}
TskBoolean tsk_array_view_const_equals(const TskType *array_view_type, TskArrayViewConst array_view_1, TskArrayViewConst array_view_2) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));
	assert(tsk_array_view_const_is_valid(array_view_type, array_view_1));
	assert(tsk_array_view_const_is_valid(array_view_type, array_view_2));
	assert(tsk_type_has_trait(tsk_array_view_const_element_type(array_view_type), TSK_TRAIT_ID_EQUATABLE));

	if (tsk_array_view_const_length(array_view_type, array_view_1) != tsk_array_view_const_length(array_view_type, array_view_2)) {
		return TSK_FALSE;
	}

	for (TskUSize i = 0; i < tsk_array_view_const_length(array_view_type, array_view_1); i++) {
		if (!tsk_trait_equatable_equals(
		        tsk_array_view_const_element_type(array_view_type),
		        tsk_array_view_const_get(array_view_type, array_view_1, i),
		        tsk_array_view_const_get(array_view_type, array_view_2, i)
		    )) {
			return TSK_FALSE;
		}
	}

	return TSK_TRUE;
}
TskEmpty tsk_array_view_const_hash(const TskType *array_view_type, TskArrayViewConst array_view, const TskType *hasher_type, TskAny *hasher) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));
	assert(tsk_array_view_const_is_valid(array_view_type, array_view));
	assert(tsk_type_has_trait(tsk_array_view_const_element_type(array_view_type), TSK_TRAIT_ID_HASHABLE));

	TskUSize length = tsk_array_view_const_length(array_view_type, array_view);
	tsk_trait_hasher_combine(
	    hasher_type,
	    hasher,
	    (const TskU8 *)&length,
	    sizeof(length)
	);

	for (TskUSize i = 0; i < tsk_array_view_const_length(array_view_type, array_view); i++) {
		tsk_trait_hashable_hash(
		    tsk_array_view_const_element_type(array_view_type),
		    tsk_array_view_const_get(array_view_type, array_view, i),
		    hasher_type,
		    hasher
		);
	}
}

TskOrdering tsk_array_view_const_type_trait_comparable_compare(const TskType *comparable_type, const TskAny *comparable_1, const TskAny *comparable_2) {
	return tsk_array_view_const_compare(comparable_type, *(const TskArrayViewConst *)comparable_1, *(const TskArrayViewConst *)comparable_2);
}
TskBoolean tsk_array_view_const_type_trait_equatable_equals(const TskType *equatable_type, const TskAny *equatable_1, const TskAny *equatable_2) {
	return tsk_array_view_const_equals(equatable_type, *(const TskArrayViewConst *)equatable_1, *(const TskArrayViewConst *)equatable_2);
}
TskEmpty tsk_array_view_const_type_trait_hashable_hash(const TskType *hashable_type, const TskAny *hashable, const TskType *hasher_type, TskAny *hasher) {
	tsk_array_view_const_hash(hashable_type, *(const TskArrayViewConst *)hashable, hasher_type, hasher);
}

const TskTraitComplete tsk_array_view_const_type_trait_complete = {
	.size      = sizeof(TskArrayViewConst),
	.alignment = alignof(TskArrayViewConst),
};
const TskTraitDroppable tsk_array_view_const_type_trait_droppable = {
	.drop = TSK_NULL,
};
const TskTraitClonable tsk_array_view_const_type_trait_clonable = {
	.clone = TSK_NULL,
};
const TskTraitComparable tsk_array_view_const_type_trait_comparable = {
	.compare = tsk_array_view_const_type_trait_comparable_compare,
};
const TskTraitEquatable tsk_array_view_const_type_trait_equatable = {
	.equals = tsk_array_view_const_type_trait_equatable_equals,
};
const TskTraitHashable tsk_array_view_const_type_trait_hashable = {
	.hash = tsk_array_view_const_type_trait_hashable_hash,
};

#define TSK_ARRAY_VIEW_CONST_TYPES_CAPACITY ((TskUSize)1 << 7)

TskArrayViewConstType tsk_array_view_const_types[TSK_ARRAY_VIEW_CONST_TYPES_CAPACITY];

TskBoolean tsk_array_view_const_type_is_valid(const TskType *array_view_type) {
	return tsk_type_is_valid(array_view_type) &&
	       &tsk_array_view_const_types[0] <= (const TskArrayViewConstType *)array_view_type && (const TskArrayViewConstType *)array_view_type < &tsk_array_view_const_types[TSK_ARRAY_VIEW_CONST_TYPES_CAPACITY];
}
const TskType *tsk_array_view_const_type(const TskType *element_type) {
	assert(tsk_type_is_valid(element_type));
	assert(tsk_type_has_trait(element_type, TSK_TRAIT_ID_COMPLETE));

	const TskType             *hasher_type = tsk_trait_builder_built_type(tsk_default_hasher_builder_type);
	alignas(max_align_t) TskU8 hasher[tsk_trait_complete_size(hasher_type)];
	tsk_trait_builder_build(tsk_default_hasher_builder_type, tsk_default_hasher_builder, hasher);

	tsk_trait_hasher_combine(hasher_type, hasher, (const TskU8 *)&element_type, sizeof(element_type)); // NOLINT(bugprone-sizeof-expression)
	TskU64 hash = tsk_trait_hasher_finalize(hasher_type, hasher);

	tsk_trait_droppable_drop(hasher_type, hasher);

	TskUSize starting_index = hash & (TSK_ARRAY_VIEW_CONST_TYPES_CAPACITY - 1);
	TskUSize index          = starting_index;
	while (tsk_array_view_const_types[index].element_type != TSK_NULL) {
		if (tsk_array_view_const_types[index].element_type == element_type) {
			return &tsk_array_view_const_types[index].array_view_const_type;
		}
		index = (index + 1) & (TSK_ARRAY_VIEW_CONST_TYPES_CAPACITY - 1);
		if (index == starting_index) {
			return TSK_NULL;
		}
	}

	tsk_array_view_const_types[index].array_view_const_type.trait_table                                                                                                                     = &tsk_array_view_const_types[index].array_view_const_type_trait_table;
	tsk_array_view_const_types[index].array_view_const_type_trait_table.entries                                                                                                             = tsk_array_view_const_types[index].array_view_const_type_trait_table_entries;
	tsk_array_view_const_types[index].array_view_const_type_trait_table.capacity                                                                                                            = sizeof(tsk_array_view_const_types[index].array_view_const_type_trait_table_entries) / sizeof(tsk_array_view_const_types[index].array_view_const_type_trait_table_entries[0]);

	tsk_array_view_const_types[index].array_view_const_type_trait_table.entries[TSK_TRAIT_ID_COMPLETE & (tsk_array_view_const_types[index].array_view_const_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
		.trait_id   = TSK_TRAIT_ID_COMPLETE,
		.trait_data = &tsk_array_view_const_type_trait_complete,
	};
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_DROPPABLE)) {
		tsk_array_view_const_types[index].array_view_const_type_trait_table.entries[TSK_TRAIT_ID_DROPPABLE & (tsk_array_view_const_types[index].array_view_const_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_DROPPABLE,
			.trait_data = &tsk_array_view_const_type_trait_droppable,
		};
	}
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_CLONABLE)) {
		tsk_array_view_const_types[index].array_view_const_type_trait_table.entries[TSK_TRAIT_ID_CLONABLE & (tsk_array_view_const_types[index].array_view_const_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_CLONABLE,
			.trait_data = &tsk_array_view_const_type_trait_clonable,
		};
	}
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_COMPARABLE)) {
		tsk_array_view_const_types[index].array_view_const_type_trait_table.entries[TSK_TRAIT_ID_COMPARABLE & (tsk_array_view_const_types[index].array_view_const_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_COMPARABLE,
			.trait_data = &tsk_array_view_const_type_trait_comparable,
		};
	}
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_EQUATABLE)) {
		tsk_array_view_const_types[index].array_view_const_type_trait_table.entries[TSK_TRAIT_ID_EQUATABLE & (tsk_array_view_const_types[index].array_view_const_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_EQUATABLE,
			.trait_data = &tsk_array_view_const_type_trait_equatable,
		};
	}
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_HASHABLE)) {
		tsk_array_view_const_types[index].array_view_const_type_trait_table.entries[TSK_TRAIT_ID_HASHABLE & (tsk_array_view_const_types[index].array_view_const_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_HASHABLE,
			.trait_data = &tsk_array_view_const_type_trait_hashable,
		};
	}

	tsk_array_view_const_types[index].element_type = element_type;

	(void)snprintf(
	    tsk_array_view_const_types[index].array_view_const_type_name,
	    sizeof(tsk_array_view_const_types[index].array_view_const_type_name),
	    "TskArrayViewConst<%s>",
	    tsk_type_name(element_type)
	);
	tsk_array_view_const_types[index].array_view_const_type.name = tsk_array_view_const_types[index].array_view_const_type_name;

	const TskType *array_view_const_type                         = &tsk_array_view_const_types[index].array_view_const_type;

	assert(tsk_array_view_const_type_is_valid(array_view_const_type));

	return array_view_const_type;
}

TskArrayViewConst tsk_array_view_const(const TskType *array_type, const TskArray *array) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));

	TskArrayViewConst array_view = {
		.elements = tsk_array_elements_const(array_type, array),
		.length   = tsk_array_length(array_type, array),
		.stride   = 1,
	};

	assert(tsk_array_view_const_is_valid(tsk_array_view_const_type(tsk_array_element_type(array_type)), array_view));

	return array_view;
}
TskArrayViewConst tsk_array_view_as_const(const TskType *array_view_type, TskArrayView array_view) {
	assert(tsk_array_view_type_is_valid(array_view_type));
	assert(tsk_array_view_is_valid(array_view_type, array_view));

	TskArrayViewConst array_view_const = {
		.elements = tsk_array_view_elements(array_view_type, array_view),
		.length   = tsk_array_view_length(array_view_type, array_view),
		.stride   = tsk_array_view_stride(array_view_type, array_view),
	};

	assert(tsk_array_view_const_is_valid(tsk_array_view_const_type(tsk_array_view_element_type(array_view_type)), array_view_const));

	return array_view_const;
}

typedef struct TskArrayIteratorType TskArrayIteratorType;
struct TskArrayIteratorType {
	TskType        array_iterator_type;
	TskCharacter   array_iterator_type_name[40];
	const TskType *element_type;
};

TskBoolean tsk_array_iterator_is_valid(const TskType *array_iterator_type, const TskArrayIterator *array_iterator) {
	assert(tsk_array_iterator_type_is_valid(array_iterator_type));

	return array_iterator != TSK_NULL && (array_iterator->elements != TSK_NULL || array_iterator->length == 0);
}
const TskType *tsk_array_iterator_element_type(const TskType *array_iterator_type) {
	assert(tsk_array_iterator_type_is_valid(array_iterator_type));

	return ((const TskArrayIteratorType *)array_iterator_type)->element_type;
}
const TskType *tsk_array_iterator_item_type(const TskType *array_iterator_type) {
	assert(tsk_array_iterator_type_is_valid(array_iterator_type));

	return tsk_reference_type(tsk_array_iterator_element_type(array_iterator_type));
}
TskBoolean tsk_array_iterator_next(const TskType *array_iterator_type, TskArrayIterator *array_iterator, TskAny *item) {
	assert(tsk_array_iterator_is_valid(array_iterator_type, array_iterator));
	assert(item != TSK_NULL);

	if (array_iterator->length == 0) {
		return TSK_FALSE;
	}

	memcpy(
	    item,
	    array_iterator->elements,
	    tsk_trait_complete_size(tsk_array_iterator_item_type(array_iterator_type))
	);

	if (array_iterator->stride > 0) {
		array_iterator->elements = (TskU8 *)array_iterator->elements + ((TskUSize)array_iterator->stride * tsk_trait_complete_size(tsk_array_iterator_element_type(array_iterator_type)));
	} else {
		array_iterator->elements = (TskU8 *)array_iterator->elements - ((TskUSize)-array_iterator->stride * tsk_trait_complete_size(tsk_array_iterator_element_type(array_iterator_type)));
	}
	array_iterator->length--;

	return TSK_TRUE;
}

const TskType *tsk_array_iterator_type_trait_iterator_item_type(const TskType *iterator_type) {
	return tsk_array_iterator_item_type(iterator_type);
}
TskBoolean tsk_array_iterator_type_trait_iterator_next(const TskType *iterator_type, TskAny *iterator, TskAny *item) {
	return tsk_array_iterator_next(iterator_type, iterator, item);
}

// clang-format off
TSK_TYPE(tsk_array_iterator_type_, TskArrayIterator,
	TSK_TYPE_TRAIT(tsk_array_iterator_type_, TSK_TRAIT_ID_COMPLETE, &(TskTraitComplete){
		.size      = sizeof(TskArrayIterator),
		.alignment = alignof(TskArrayIterator),
	}),
	TSK_TYPE_TRAIT(tsk_array_iterator_type_, TSK_TRAIT_ID_DROPPABLE, &(TskTraitDroppable){
		.drop = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_array_iterator_type_, TSK_TRAIT_ID_CLONABLE, &(TskTraitClonable){
		.clone = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_array_iterator_type_, TSK_TRAIT_ID_EQUATABLE, &(TskTraitEquatable){
		.equals = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_array_iterator_type_, TSK_TRAIT_ID_ITERATOR, &(TskTraitIterator){
		.item_type = tsk_array_iterator_type_trait_iterator_item_type,
		.next      = tsk_array_iterator_type_trait_iterator_next,
	}),
);
// clang-format on

#define TSK_ARRAY_ITERATOR_TYPES_CAPACITY ((TskUSize)1 << 7)

TskArrayIteratorType tsk_array_iterator_types[TSK_ARRAY_ITERATOR_TYPES_CAPACITY];

TskBoolean tsk_array_iterator_type_is_valid(const TskType *array_iterator_type) {
	return tsk_type_is_valid(array_iterator_type) &&
	       &tsk_array_iterator_types[0] <= (const TskArrayIteratorType *)array_iterator_type && (const TskArrayIteratorType *)array_iterator_type < &tsk_array_iterator_types[TSK_ARRAY_ITERATOR_TYPES_CAPACITY];
}
const TskType *tsk_array_iterator_type(const TskType *element_type) {
	assert(tsk_array_type_is_valid(element_type));
	assert(tsk_type_has_trait(element_type, TSK_TRAIT_ID_COMPLETE));

	const TskType             *hasher_type = tsk_trait_builder_built_type(tsk_default_hasher_builder_type);
	alignas(max_align_t) TskU8 hasher[tsk_trait_complete_size(hasher_type)];
	tsk_trait_builder_build(tsk_default_hasher_builder_type, tsk_default_hasher_builder, hasher);

	tsk_trait_hasher_combine(hasher_type, hasher, (const TskU8 *)&element_type, sizeof(element_type)); // NOLINT(bugprone-sizeof-expression)
	TskU64 hash = tsk_trait_hasher_finalize(hasher_type, hasher);

	tsk_trait_droppable_drop(hasher_type, hasher);

	TskUSize starting_index = hash & (TSK_ARRAY_ITERATOR_TYPES_CAPACITY - 1);
	TskUSize index          = starting_index;
	while (tsk_array_iterator_types[index].element_type != TSK_NULL) {
		if (tsk_array_iterator_types[index].element_type == element_type) {
			return &tsk_array_iterator_types[index].array_iterator_type;
		}
		index = (index + 1) & (TSK_ARRAY_ITERATOR_TYPES_CAPACITY - 1);
		if (index == starting_index) {
			return TSK_NULL;
		}
	}

	tsk_array_iterator_types[index].array_iterator_type = *tsk_array_iterator_type_;
	tsk_array_iterator_types[index].element_type        = element_type;

	(void)snprintf(
	    tsk_array_iterator_types[index].array_iterator_type_name,
	    sizeof(tsk_array_iterator_types[index].array_iterator_type_name),
	    "TskArrayIterator<%s>",
	    tsk_type_name(element_type)
	);
	tsk_array_iterator_types[index].array_iterator_type.name = tsk_array_iterator_types[index].array_iterator_type_name;

	const TskType *array_iterator_type                       = &tsk_array_iterator_types[index].array_iterator_type;

	assert(tsk_array_iterator_type_is_valid(array_iterator_type));

	return array_iterator_type;
}

TskArrayIterator tsk_array_iterator(const TskType *array_type, TskArray *array) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));

	TskArrayIterator array_iterator = {
		.elements = tsk_array_elements(array_type, array),
		.length   = tsk_array_length(array_type, array),
		.stride   = 1,
	};

	assert(tsk_array_iterator_is_valid(tsk_array_iterator_type(array_type), &array_iterator));

	return array_iterator;
}
TskArrayIterator tsk_array_view_iterator(const TskType *array_view_type, TskArrayView array_view) {
	assert(tsk_array_view_type_is_valid(array_view_type));
	assert(tsk_array_view_is_valid(array_view_type, array_view));

	TskArrayIterator array_iterator = {
		.elements = tsk_array_view_elements(array_view_type, array_view),
		.length   = tsk_array_view_length(array_view_type, array_view),
		.stride   = tsk_array_view_stride(array_view_type, array_view)
	};

	assert(tsk_array_iterator_is_valid(tsk_array_iterator_type(tsk_array_view_element_type(array_view_type)), &array_iterator));

	return array_iterator;
}

typedef struct TskArrayIteratorConstType TskArrayIteratorConstType;
struct TskArrayIteratorConstType {
	TskType        array_iterator_const_type;
	TskCharacter   array_iterator_const_type_name[40];
	const TskType *element_type;
};

TskBoolean tsk_array_iterator_const_is_valid(const TskType *array_iterator_type, const TskArrayIteratorConst *array_iterator) {
	assert(tsk_array_iterator_const_type_is_valid(array_iterator_type));

	return array_iterator != TSK_NULL && (array_iterator->elements != TSK_NULL || array_iterator->length == 0);
}
const TskType *tsk_array_iterator_const_element_type(const TskType *array_iterator_type) {
	assert(tsk_array_iterator_const_type_is_valid(array_iterator_type));

	return ((const TskArrayIteratorConstType *)array_iterator_type)->element_type;
}
const TskType *tsk_array_iterator_const_item_type(const TskType *array_iterator_type) {
	assert(tsk_array_iterator_const_type_is_valid(array_iterator_type));

	return tsk_reference_const_type(tsk_array_iterator_const_element_type(array_iterator_type));
}
TskBoolean tsk_array_iterator_const_next(const TskType *array_iterator_type, TskArrayIteratorConst *array_iterator, TskAny *item) {
	assert(tsk_array_iterator_const_is_valid(array_iterator_type, array_iterator));
	assert(item != TSK_NULL);

	if (array_iterator->length == 0) {
		return TSK_FALSE;
	}

	memcpy(
	    item,
	    array_iterator->elements,
	    tsk_trait_complete_size(tsk_array_iterator_const_item_type(array_iterator_type))
	);

	if (array_iterator->stride > 0) {
		array_iterator->elements = (const TskU8 *)array_iterator->elements + ((TskUSize)array_iterator->stride * tsk_trait_complete_size(tsk_array_iterator_const_element_type(array_iterator_type)));
	} else {
		array_iterator->elements = (const TskU8 *)array_iterator->elements - ((TskUSize)-array_iterator->stride * tsk_trait_complete_size(tsk_array_iterator_const_element_type(array_iterator_type)));
	}
	array_iterator->length--;

	return TSK_TRUE;
}

const TskType *tsk_array_iterator_const_type_trait_iterator_item_type(const TskType *iterator_type) {
	return tsk_array_iterator_const_item_type(iterator_type);
}
TskBoolean tsk_array_iterator_const_type_trait_iterator_next(const TskType *iterator_type, TskAny *iterator, TskAny *item) {
	return tsk_array_iterator_const_next(iterator_type, iterator, item);
}

// clang-format off
TSK_TYPE(tsk_array_iterator_const_type_, TskArrayIteratorConst,
	TSK_TYPE_TRAIT(tsk_array_iterator_const_type_, TSK_TRAIT_ID_COMPLETE, &(TskTraitComplete){
		.size      = sizeof(TskArrayIteratorConst),
		.alignment = alignof(TskArrayIteratorConst),
	}),
	TSK_TYPE_TRAIT(tsk_array_iterator_const_type_, TSK_TRAIT_ID_DROPPABLE, &(TskTraitDroppable){
		.drop = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_array_iterator_const_type_, TSK_TRAIT_ID_CLONABLE, &(TskTraitClonable){
		.clone = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_array_iterator_const_type_, TSK_TRAIT_ID_EQUATABLE, &(TskTraitEquatable){
		.equals = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_array_iterator_const_type_, TSK_TRAIT_ID_ITERATOR, &(TskTraitIterator){
		.item_type = tsk_array_iterator_const_type_trait_iterator_item_type,
		.next      = tsk_array_iterator_const_type_trait_iterator_next,
	}),
);
// clang-format on

#define TSK_ARRAY_ITERATOR_CONST_TYPES_CAPACITY ((TskUSize)1 << 7)

TskArrayIteratorConstType tsk_array_iterator_const_types[TSK_ARRAY_ITERATOR_CONST_TYPES_CAPACITY];

TskBoolean tsk_array_iterator_const_type_is_valid(const TskType *array_iterator_type) {
	return tsk_type_is_valid(array_iterator_type) &&
	       &tsk_array_iterator_const_types[0] <= (const TskArrayIteratorConstType *)array_iterator_type && (const TskArrayIteratorConstType *)array_iterator_type < &tsk_array_iterator_const_types[TSK_ARRAY_ITERATOR_CONST_TYPES_CAPACITY];
}
const TskType *tsk_array_iterator_const_type(const TskType *element_type) {
	assert(tsk_array_type_is_valid(element_type));
	assert(tsk_type_has_trait(element_type, TSK_TRAIT_ID_COMPLETE));

	const TskType             *hasher_type = tsk_trait_builder_built_type(tsk_default_hasher_builder_type);
	alignas(max_align_t) TskU8 hasher[tsk_trait_complete_size(hasher_type)];
	tsk_trait_builder_build(tsk_default_hasher_builder_type, tsk_default_hasher_builder, hasher);

	tsk_trait_hasher_combine(hasher_type, hasher, (const TskU8 *)&element_type, sizeof(element_type)); // NOLINT(bugprone-sizeof-expression)
	TskU64 hash = tsk_trait_hasher_finalize(hasher_type, hasher);

	tsk_trait_droppable_drop(hasher_type, hasher);

	TskUSize starting_index = hash & (TSK_ARRAY_ITERATOR_CONST_TYPES_CAPACITY - 1);
	TskUSize index          = starting_index;
	while (tsk_array_iterator_const_types[index].element_type != TSK_NULL) {
		if (tsk_array_iterator_const_types[index].element_type == element_type) {
			return &tsk_array_iterator_const_types[index].array_iterator_const_type;
		}
		index = (index + 1) & (TSK_ARRAY_ITERATOR_CONST_TYPES_CAPACITY - 1);
		if (index == starting_index) {
			return TSK_NULL;
		}
	}

	tsk_array_iterator_const_types[index].array_iterator_const_type = *tsk_array_iterator_const_type_;
	tsk_array_iterator_const_types[index].element_type              = element_type;

	(void)snprintf(
	    tsk_array_iterator_const_types[index].array_iterator_const_type_name,
	    sizeof(tsk_array_iterator_const_types[index].array_iterator_const_type_name),
	    "TskArrayIteratorConst<%s>",
	    tsk_type_name(element_type)
	);
	tsk_array_iterator_const_types[index].array_iterator_const_type.name = tsk_array_iterator_const_types[index].array_iterator_const_type_name;

	const TskType *array_iterator_const_type                             = &tsk_array_iterator_const_types[index].array_iterator_const_type;

	assert(tsk_array_iterator_const_type_is_valid(array_iterator_const_type));

	return array_iterator_const_type;
}

TskArrayIteratorConst tsk_array_iterator_const(const TskType *array_type, const TskArray *array) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));

	TskArrayIteratorConst array_iterator = {
		.elements = tsk_array_elements_const(array_type, array),
		.length   = tsk_array_length(array_type, array),
		.stride   = 1,
	};

	assert(tsk_array_iterator_const_is_valid(tsk_array_iterator_const_type(array_type), &array_iterator));

	return array_iterator;
}
TskArrayIteratorConst tsk_array_view_const_iterator(const TskType *array_view_type, TskArrayViewConst array_view) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));
	assert(tsk_array_view_const_is_valid(array_view_type, array_view));

	TskArrayIteratorConst array_iterator = {
		.elements = tsk_array_view_const_elements(array_view_type, array_view),
		.length   = tsk_array_view_const_length(array_view_type, array_view),
		.stride   = tsk_array_view_const_stride(array_view_type, array_view)
	};

	assert(tsk_array_iterator_const_is_valid(tsk_array_iterator_const_type(tsk_array_view_const_element_type(array_view_type)), &array_iterator));

	return array_iterator;
}
