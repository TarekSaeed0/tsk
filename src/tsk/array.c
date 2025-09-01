#include <tsk/array.h>

#include <tsk/default_hasher.h>
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array_1));
	assert(tsk_array_is_valid(array_type, array_2));
	assert(tsk_type_has_trait(tsk_array_element_type(array_type), TSK_TRAIT_ID_COMPARABLE));

	for (TskUSize i = 0; i < tsk_array_length(array_type, array_1) && i < tsk_array_length(array_type, array_2); i++) {
		TskOrdering ordering = tsk_trait_comparable_compare(
		    tsk_array_element_type(array_type),
		    tsk_array_get_const(array_type, array_1, i),
		    tsk_array_get_const(array_type, array_2, i)
		);
		if (ordering != TSK_ORDERING_EQUAL) {
			return ordering;
		}
	}

	return (tsk_array_length(array_type, array_1) > tsk_array_length(array_type, array_2)) -
	       (tsk_array_length(array_type, array_1) < tsk_array_length(array_type, array_2));
}
TskBoolean tsk_array_equals(const TskType *array_type, const TskArray *array_1, const TskArray *array_2) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array_1));
	assert(tsk_array_is_valid(array_type, array_2));
	assert(tsk_type_has_trait(tsk_array_element_type(array_type), TSK_TRAIT_ID_EQUATABLE));

	if (tsk_array_length(array_type, array_1) != tsk_array_length(array_type, array_2)) {
		return TSK_FALSE;
	}

	for (TskUSize i = 0; i < tsk_array_length(array_type, array_1); i++) {
		if (!tsk_trait_equatable_equals(
		        tsk_array_element_type(array_type),
		        tsk_array_get_const(array_type, array_1, i),
		        tsk_array_get_const(array_type, array_2, i)
		    )) {
			return TSK_FALSE;
		}
	}

	return TSK_TRUE;
}
TskEmpty tsk_array_hash(const TskType *array_type, const TskArray *array, const TskType *hasher_type, TskAny *hasher) {
	assert(tsk_array_type_is_valid(array_type));
	assert(tsk_array_is_valid(array_type, array));
	assert(tsk_type_has_trait(tsk_array_element_type(array_type), TSK_TRAIT_ID_COMPARABLE));
	assert(tsk_type_is_valid(hasher_type));
	assert(tsk_type_has_trait(hasher_type, TSK_TRAIT_ID_HASHER));
	assert(hasher != TSK_NULL);

	TskUSize length = tsk_array_length(array_type, array);
	tsk_trait_hasher_combine(
	    hasher_type,
	    hasher,
	    (const TskU8 *)&length,
	    sizeof(length)
	);

	for (TskUSize i = 0; i < tsk_array_length(array_type, array); i++) {
		tsk_trait_hashable_hash(
		    tsk_array_element_type(array_type),
		    tsk_array_get_const(array_type, array, i),
		    hasher_type,
		    hasher
		);
	}
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

/* TskBoolean tsk_array_view_is_valid(TskArrayView array_view) {
  return array_view.elements != TSK_NULL && tsk_type_is_valid(array_view.element_type) &&
         tsk_type_has_trait(array_view.element_type, TSK_TRAIT_COMPLETE) && (array_view.elements != TSK_NULL || array_view.length == 0) && array_view.length <= SIZE_MAX / tsk_trait_complete_size(tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPLETE));
}
TskArrayView tsk_array_view_slice(TskArrayView array_view, TskUSize start, TskUSize end, TskISize step) {
  assert(tsk_array_view_is_valid(array_view));
  assert(step != 0);

  if (step > 0) {
    assert(end <= tsk_array_view_length(array_view));
    assert(start <= end);

    TskArrayView array_view_slice = {
      .element_type = tsk_array_view_element_type(array_view),
      .elements     = (TskU8 *)array_view.elements + (start * tsk_trait_complete_size(tsk_type_trait(tsk_array_view_element_type(array_view), TSK_TRAIT_COMPLETE))),
      .length       = (end - start + (TskUSize)step - 1) / (TskUSize)step,
      .stride       = array_view.stride * step,
    };

    assert(tsk_array_view_is_valid(array_view_slice));

    return array_view_slice;
  }

  assert(start <= tsk_array_view_length(array_view));
  assert(end <= start);

  TskArrayView array_view_slice = {
    .element_type = tsk_array_view_element_type(array_view),
    .elements     = (TskU8 *)array_view.elements + (start * tsk_trait_complete_size(tsk_type_trait(tsk_array_view_element_type(array_view), TSK_TRAIT_COMPLETE))) - tsk_trait_complete_size(tsk_type_trait(tsk_array_view_element_type(array_view), TSK_TRAIT_COMPLETE)),
    .length       = (start - end + (TskUSize)-step - 1) / (TskUSize)-step,
    .stride       = array_view.stride * step,
  };

  assert(tsk_array_view_is_valid(array_view_slice));

  return array_view_slice;
}
const TskType *tsk_array_view_element_type(TskArrayView array_view) {
  assert(tsk_array_view_is_valid(array_view));

  return array_view.element_type;
}
TskUSize tsk_array_view_length(TskArrayView array_view) {
  assert(tsk_array_view_is_valid(array_view));

  return array_view.length;
}
TskBoolean tsk_array_view_is_empty(TskArrayView array_view) {
  assert(tsk_array_view_is_valid(array_view));

  return tsk_array_view_length(array_view) == 0;
}
TskISize tsk_array_view_stride(TskArrayView array_view) {
  assert(tsk_array_view_is_valid(array_view));

  return array_view.stride;
}
TskAny *tsk_array_view_get(TskArrayView array_view, TskUSize index) {
  assert(tsk_array_view_is_valid(array_view));
  assert(index < tsk_array_view_length(array_view));

  if (array_view.stride > 0) {
    return (TskU8 *)array_view.elements + (index * (TskUSize)array_view.stride * tsk_trait_complete_size(tsk_type_trait(tsk_array_view_element_type(array_view), TSK_TRAIT_COMPLETE)));
  }

  return (TskU8 *)array_view.elements - (index * (TskUSize)-array_view.stride * tsk_trait_complete_size(tsk_type_trait(tsk_array_view_element_type(array_view), TSK_TRAIT_COMPLETE)));
}
TskAny *tsk_array_view_front(TskArrayView array_view) {
  assert(tsk_array_view_is_valid(array_view));

  if (tsk_array_view_is_empty(array_view)) {
    return TSK_NULL;
  }

  return tsk_array_view_get(array_view, 0);
}
TskAny *tsk_array_view_back(TskArrayView array_view) {
  assert(tsk_array_view_is_valid(array_view));

  if (tsk_array_view_is_empty(array_view)) {
    return TSK_NULL;
  }

  return tsk_array_view_get(array_view, tsk_array_view_length(array_view) - 1);
}
TskEmpty tsk_array_view_swap(TskArrayView array_view, TskUSize index_1, TskUSize index_2) {
  assert(tsk_array_view_is_valid(array_view));
  assert(index_1 < tsk_array_view_length(array_view));
  assert(index_2 < tsk_array_view_length(array_view));

  if (index_1 == index_2) {
    return;
  }

  TskU8 *elements_1 = (TskU8 *)tsk_array_view_get(array_view, index_1);
  TskU8 *elements_2 = (TskU8 *)tsk_array_view_get(array_view, index_2);
  for (TskUSize i = 0; i < tsk_trait_complete_size(tsk_type_trait(tsk_array_view_element_type(array_view), TSK_TRAIT_COMPLETE)); i++) {
    TskU8 byte    = elements_1[i];
    elements_1[i] = elements_2[i];
    elements_2[i] = byte;
  }
}
TskBoolean tsk_array_view_linear_search(TskArrayView array_view, const TskAny *element, TskUSize *index) {
  return tsk_array_view_const_binary_search(tsk_array_view_to_const(array_view), element, index);
}
TskBoolean tsk_array_view_binary_search(TskArrayView array_view, const TskAny *element, TskUSize *index) {
  return tsk_array_view_const_binary_search(tsk_array_view_to_const(array_view), element, index);
}
TskUSize tsk_array_view_lower_bound(TskArrayView array_view, const TskAny *element) {
  return tsk_array_view_const_lower_bound(tsk_array_view_to_const(array_view), element);
}
TskUSize tsk_array_view_upper_bound(TskArrayView array_view, const TskAny *element) {
  return tsk_array_view_const_upper_bound(tsk_array_view_to_const(array_view), element);
}
TskEmpty tsk_array_view_equal_range(TskArrayView array_view, const TskAny *element, TskUSize *start, TskUSize *end) {
  tsk_array_view_const_equal_range(tsk_array_view_to_const(array_view), element, start, end);
}
TskEmpty tsk_array_view_sort(TskArrayView array_view) {
  assert(tsk_array_view_is_valid(array_view));
  assert(tsk_type_has_trait(tsk_array_view_element_type(array_view), TSK_TRAIT_COMPARABLE));

  // PERF: find a way to sort, without allocating a buffer and copying the elements

  alignas(max_align_t) TskU8 buffer[1028];

  TskUSize elements_size = tsk_array_view_length(array_view) * tsk_trait_complete_size(tsk_type_trait(tsk_array_view_element_type(array_view), TSK_TRAIT_COMPLETE));
  TskAny  *elements      = TSK_NULL;
  if (elements_size > sizeof(buffer)) {
    elements = malloc(elements_size);
    assert(elements != TSK_NULL);
  } else {
    elements = buffer;
  }

  for (TskUSize i = 0; i < tsk_array_view_length(array_view); i++) {
    memcpy(
        (TskU8 *)elements + (i * tsk_trait_complete_size(tsk_type_trait(tsk_array_view_element_type(array_view), TSK_TRAIT_COMPLETE))),
        tsk_array_view_get(array_view, i),
        tsk_trait_complete_size(tsk_type_trait(tsk_array_view_element_type(array_view), TSK_TRAIT_COMPLETE))
    );
  }

  qsort(
      elements,
      tsk_array_view_const_length(tsk_array_view_to_const(array_view)),
      tsk_trait_complete_size(tsk_type_trait(tsk_array_view_element_type(array_view), TSK_TRAIT_COMPLETE)),
      ((const TskTraitComparable *)tsk_type_trait(tsk_array_view_element_type(array_view), TSK_TRAIT_COMPARABLE))->compare
  );

  for (TskUSize i = 0; i < tsk_array_view_length(array_view); i++) {
    memcpy(
        tsk_array_view_get(array_view, i),
        (TskU8 *)elements + (i * tsk_trait_complete_size(tsk_type_trait(tsk_array_view_element_type(array_view), TSK_TRAIT_COMPLETE))),
        tsk_trait_complete_size(tsk_type_trait(tsk_array_view_element_type(array_view), TSK_TRAIT_COMPLETE))
    );
  }

  if (elements_size > sizeof(buffer)) {
    free(elements);
  }
}
TskBoolean tsk_array_view_is_sorted(TskArrayView array_view) {
  return tsk_array_view_const_is_sorted(tsk_array_view_to_const(array_view));
}
TskUSize tsk_array_view_sorted_until(TskArrayView array_view) {
  return tsk_array_view_const_sorted_until(tsk_array_view_to_const(array_view));
}
TskEmpty tsk_array_view_partition(TskArrayView array_view, TskBoolean (*predicate)(const TskAny *element)) {
  assert(tsk_array_view_is_valid(array_view));
  assert(predicate != TSK_NULL);

  TskUSize i = 0;
  for (; i < tsk_array_view_length(array_view); i++) {
    if (!predicate(tsk_array_view_get(array_view, i))) {
      break;
    }
  }

  for (TskUSize j = i + 1; j < tsk_array_view_length(array_view); j++) {
    if (predicate(tsk_array_view_get(array_view, j))) {
      tsk_array_view_swap(array_view, i, j);
      i++;
    }
  }
}
TskBoolean tsk_array_view_is_partitioned(TskArrayView array_view, TskBoolean (*predicate)(const TskAny *element)) {
  return tsk_array_view_const_is_partitioned(tsk_array_view_to_const(array_view), predicate);
}
TskUSize tsk_array_view_partition_point(TskArrayView array_view, TskBoolean (*predicate)(const TskAny *element)) {
  return tsk_array_view_const_partition_point(tsk_array_view_to_const(array_view), predicate);
}
TskEmpty tsk_array_view_reverse(TskArrayView array_view) {
  assert(tsk_array_view_is_valid(array_view));

  for (TskUSize i = 0; i < tsk_array_view_length(array_view) / 2; i++) {
    tsk_array_view_swap(array_view, i, tsk_array_view_length(array_view) - 1 - i);
  }
}
TskOrdering tsk_array_view_compare(TskArrayView array_view_1, TskArrayView array_view_2) {
  return tsk_array_view_const_compare(
      tsk_array_view_to_const(array_view_1),
      tsk_array_view_to_const(array_view_2)
  );
}
TskBoolean tsk_array_view_equals(TskArrayView array_view_1, TskArrayView array_view_2) {
  return tsk_array_view_const_equals(
      tsk_array_view_to_const(array_view_1),
      tsk_array_view_to_const(array_view_2)
  );
}
TskEmpty tsk_array_view_hash(TskArrayView array_view, const TskTraitHasher *trait_hasher, TskAny *hasher) {
  tsk_array_view_const_hash(tsk_array_view_to_const(array_view), trait_hasher, hasher);
}

static const TskTraitComplete tsk_array_view_trait_complete = {
  .size      = sizeof(TskArrayView),
  .alignment = alignof(TskArrayView),
};

static const TskTraitDroppable tsk_array_view_trait_droppable = {
  .drop = TSK_NULL,
};

static const TskTraitClonable tsk_array_view_trait_clonable = {
  .trait_complete = &tsk_array_view_trait_complete,
  .clone          = TSK_NULL,
};

static TskOrdering tsk_array_view_trait_comparable_compare(const TskAny *array_view_1, const TskAny *array_view_2) {
  return tsk_array_view_compare(*(const TskArrayView *)array_view_1, *(const TskArrayView *)array_view_2);
}
static const TskTraitComparable tsk_array_view_trait_comparable = {
  .compare = tsk_array_view_trait_comparable_compare,
};

static TskBoolean tsk_array_view_type_equatable_equals(const TskAny *array_view_1, const TskAny *array_view_2) {
  return tsk_array_view_equals(*(const TskArrayView *)array_view_1, *(const TskArrayView *)array_view_2);
}
static const TskTraitEquatable tsk_array_view_trait_equatable = {
  .equals = tsk_array_view_type_equatable_equals,
};

static TskEmpty tsk_array_view_trait_hashable_hash(const TskAny *array_view, const TskTraitHasher *trait_hasher, TskAny *hasher) {
  tsk_array_view_hash(*(const TskArrayView *)array_view, trait_hasher, hasher);
}
static const TskTraitHashable tsk_array_view_trait_hashable = {
  .hash = tsk_array_view_trait_hashable_hash,
};

const TskType tsk_array_view_type = {
  .name   = "TskArrayView",
  .traits = {
      [TSK_TRAIT_COMPLETE]   = &tsk_array_view_trait_complete,
      [TSK_TRAIT_DROPPABLE]  = &tsk_array_view_trait_droppable,
      [TSK_TRAIT_CLONABLE]   = &tsk_array_view_trait_clonable,
      [TSK_TRAIT_COMPARABLE] = &tsk_array_view_trait_comparable,
      [TSK_TRAIT_EQUATABLE]  = &tsk_array_view_trait_equatable,
      [TSK_TRAIT_HASHABLE]   = &tsk_array_view_trait_hashable,
  },
};

TskBoolean tsk_array_view_const_is_valid(TskArrayViewConst array_view) {
  return array_view.elements != TSK_NULL && tsk_type_is_valid(array_view.element_type) &&
         tsk_type_has_trait(array_view.element_type, TSK_TRAIT_COMPLETE) && (array_view.elements != TSK_NULL || array_view.length == 0) && array_view.length <= SIZE_MAX / tsk_trait_complete_size(tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPLETE));
}
TskArrayViewConst tsk_array_view_const_slice(TskArrayViewConst array_view, TskUSize start, TskUSize end, TskISize step) {
  assert(tsk_array_view_const_is_valid(array_view));
  assert(step != 0);

  if (step > 0) {
    assert(end <= tsk_array_view_const_length(array_view));
    assert(start <= end);

    TskArrayViewConst array_view_slice = {
      .element_type = tsk_array_view_const_element_type(array_view),
      .elements     = (const TskU8 *)array_view.elements + (start * tsk_trait_complete_size(tsk_type_trait(tsk_array_view_const_element_type(array_view), TSK_TRAIT_COMPLETE))),
      .length       = (end - start + (TskUSize)step - 1) / (TskUSize)step,
      .stride       = array_view.stride * step,
    };

    assert(tsk_array_view_const_is_valid(array_view_slice));

    return array_view_slice;
  }

  assert(start <= tsk_array_view_const_length(array_view));
  assert(end <= start);

  TskArrayViewConst array_view_slice = {
    .element_type = tsk_array_view_const_element_type(array_view),
    .elements     = (const TskU8 *)array_view.elements + (start * tsk_trait_complete_size(tsk_type_trait(tsk_array_view_const_element_type(array_view), TSK_TRAIT_COMPLETE))) - tsk_trait_complete_size(tsk_type_trait(tsk_array_view_const_element_type(array_view), TSK_TRAIT_COMPLETE)),
    .length       = (start - end + (TskUSize)-step - 1) / (TskUSize)-step,
    .stride       = array_view.stride * step,
  };

  assert(tsk_array_view_const_is_valid(array_view_slice));

  return array_view_slice;
}
const TskType *tsk_array_view_const_element_type(TskArrayViewConst array_view) {
  assert(tsk_array_view_const_is_valid(array_view));

  return array_view.element_type;
}
TskUSize tsk_array_view_const_length(TskArrayViewConst array_view) {
  assert(tsk_array_view_const_is_valid(array_view));

  return array_view.length;
}
TskBoolean tsk_array_view_const_is_empty(TskArrayViewConst array_view) {
  assert(tsk_array_view_const_is_valid(array_view));

  return tsk_array_view_const_length(array_view) == 0;
}
TskISize tsk_array_view_const_stride(TskArrayViewConst array_view) {
  assert(tsk_array_view_const_is_valid(array_view));

  return array_view.stride;
}
const TskAny *tsk_array_view_const_get(TskArrayViewConst array_view, TskUSize index) {
  assert(tsk_array_view_const_is_valid(array_view));
  assert(index < tsk_array_view_const_length(array_view));

  if (array_view.stride > 0) {
    return (const TskU8 *)array_view.elements + (index * (TskUSize)array_view.stride * tsk_trait_complete_size(tsk_type_trait(tsk_array_view_const_element_type(array_view), TSK_TRAIT_COMPLETE)));
  }

  return (const TskU8 *)array_view.elements - (index * (TskUSize)-array_view.stride * tsk_trait_complete_size(tsk_type_trait(tsk_array_view_const_element_type(array_view), TSK_TRAIT_COMPLETE)));
}
const TskAny *tsk_array_view_const_front(TskArrayViewConst array_view) {
  assert(tsk_array_view_const_is_valid(array_view));

  if (tsk_array_view_const_is_empty(array_view)) {
    return TSK_NULL;
  }

  return tsk_array_view_const_get(array_view, 0);
}
const TskAny *tsk_array_view_const_back(TskArrayViewConst array_view) {
  assert(tsk_array_view_const_is_valid(array_view));

  if (tsk_array_view_const_is_empty(array_view)) {
    return TSK_NULL;
  }

  return tsk_array_view_const_get(array_view, tsk_array_view_const_length(array_view) - 1);
}
TskBoolean tsk_array_view_const_linear_search(TskArrayViewConst array_view, const TskAny *element, TskUSize *index) {
  assert(tsk_array_view_const_is_valid(array_view));
  assert(element != TSK_NULL);
  assert(tsk_type_has_trait(tsk_array_view_const_element_type(array_view), TSK_TRAIT_EQUATABLE));

  for (TskUSize i = 0; i < tsk_array_view_const_length(array_view); i++) {
    if (tsk_trait_equatable_equals(
            tsk_type_trait(tsk_array_view_const_element_type(array_view), TSK_TRAIT_EQUATABLE),
            tsk_array_view_const_get(array_view, i),
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
TskBoolean tsk_array_view_const_binary_search(TskArrayViewConst array_view, const TskAny *element, TskUSize *index) {
  assert(tsk_array_view_const_is_valid(array_view));
  assert(element != TSK_NULL);
  assert(tsk_type_has_trait(tsk_array_view_const_element_type(array_view), TSK_TRAIT_COMPARABLE));

  TskUSize left  = 0;
  TskUSize right = tsk_array_view_const_length(array_view) - 1;
  while (left <= right) {
    TskUSize    middle   = left + ((right - left) / 2);
    TskOrdering ordering = tsk_trait_comparable_compare(
        tsk_type_trait(tsk_array_view_const_element_type(array_view), TSK_TRAIT_COMPARABLE),
        tsk_array_view_const_get(array_view, middle),
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
TskUSize tsk_array_view_const_lower_bound(TskArrayViewConst array_view, const TskAny *element) {
  assert(tsk_array_view_const_is_valid(array_view));
  assert(element != TSK_NULL);
  assert(tsk_type_has_trait(tsk_array_view_const_element_type(array_view), TSK_TRAIT_COMPARABLE));

  TskUSize left  = 0;
  TskUSize right = tsk_array_view_const_length(array_view);
  TskUSize index = tsk_array_view_const_length(array_view);
  while (left < right) {
    TskUSize    middle   = left + ((right - left) / 2);
    TskOrdering ordering = tsk_trait_comparable_compare(
        tsk_type_trait(tsk_array_view_const_element_type(array_view), TSK_TRAIT_COMPARABLE),
        tsk_array_view_const_get(array_view, middle),
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
TskUSize tsk_array_view_const_upper_bound(TskArrayViewConst array_view, const TskAny *element) {
  assert(tsk_array_view_const_is_valid(array_view));
  assert(element != TSK_NULL);
  assert(tsk_type_has_trait(tsk_array_view_const_element_type(array_view), TSK_TRAIT_COMPARABLE));

  TskUSize left  = 0;
  TskUSize right = tsk_array_view_const_length(array_view);
  TskUSize index = tsk_array_view_const_length(array_view);
  while (left < right) {
    TskUSize    middle   = left + ((right - left) / 2);
    TskOrdering ordering = tsk_trait_comparable_compare(
        tsk_type_trait(tsk_array_view_const_element_type(array_view), TSK_TRAIT_COMPARABLE),
        tsk_array_view_const_get(array_view, middle),
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
TskEmpty tsk_array_view_const_equal_range(TskArrayViewConst array_view, const TskAny *element, TskUSize *start, TskUSize *end) {
  assert(tsk_array_view_const_is_valid(array_view));
  assert(element != TSK_NULL);
  assert(tsk_type_has_trait(tsk_array_view_const_element_type(array_view), TSK_TRAIT_COMPARABLE));
  assert(start != TSK_NULL);
  assert(end != TSK_NULL);

  *start = tsk_array_view_const_lower_bound(array_view, element);
  *end   = *start + tsk_array_view_const_upper_bound(
                      tsk_array_view_const_slice(
                          array_view,
                          *start,
                          tsk_array_view_const_length(array_view),
                          1
                      ),
                      element
                  );
}
TskBoolean tsk_array_view_const_is_sorted(TskArrayViewConst array_view) {
  return tsk_array_view_const_sorted_until(array_view) == tsk_array_view_const_length(array_view);
}
TskUSize tsk_array_view_const_sorted_until(TskArrayViewConst array_view) {
  assert(tsk_array_view_const_is_valid(array_view));
  assert(tsk_type_has_trait(tsk_array_view_const_element_type(array_view), TSK_TRAIT_COMPARABLE));

  for (TskUSize i = 1; i < tsk_array_view_const_length(array_view); i++) {
    TskOrdering ordering = tsk_trait_comparable_compare(
        tsk_type_trait(tsk_array_view_const_element_type(array_view), TSK_TRAIT_COMPARABLE),
        tsk_array_view_const_get(array_view, i - 1),
        tsk_array_view_const_get(array_view, i)
    );
    if (ordering > 0) {
      return i;
    }
  }

  return tsk_array_view_const_length(array_view);
}
TskBoolean tsk_array_view_const_is_partitioned(TskArrayViewConst array_view, TskBoolean (*predicate)(const TskAny *element)) {
  TskUSize i = 0;
  for (; i < tsk_array_view_const_length(array_view); i++) {
    if (!predicate(tsk_array_view_const_get(array_view, i))) {
      break;
    }
  }

  for (i++; i < tsk_array_view_const_length(array_view); i++) {
    if (predicate(tsk_array_view_const_get(array_view, i))) {
      return TSK_FALSE;
    }
  }

  return TSK_TRUE;
}
TskUSize tsk_array_view_const_partition_point(TskArrayViewConst array_view, TskBoolean (*predicate)(const TskAny *element)) {
  assert(tsk_array_view_const_is_valid(array_view));
  assert(predicate != TSK_NULL);

  TskUSize left  = 0;
  TskUSize right = tsk_array_view_const_length(array_view) - 1;
  TskUSize index = tsk_array_view_const_length(array_view);
  while (left <= right) {
    TskUSize middle = left + ((right - left) / 2);
    if (predicate(tsk_array_view_const_get(array_view, middle))) {
      left = middle + 1;
    } else {
      index = middle;
      right = middle - 1;
    }
  }

  return index;
}
TskOrdering tsk_array_view_const_compare(TskArrayViewConst array_view_1, TskArrayViewConst array_view_2) {
  assert(tsk_array_view_const_is_valid(array_view_1));
  assert(tsk_array_view_const_is_valid(array_view_2));
  assert(tsk_array_view_const_element_type(array_view_1) == tsk_array_view_const_element_type(array_view_2));
  assert(tsk_type_has_trait(tsk_array_view_const_element_type(array_view_1), TSK_TRAIT_COMPARABLE));

  for (TskUSize i = 0; i < tsk_array_view_const_length(array_view_1) && i < tsk_array_view_const_length(array_view_2); i++) {
    TskOrdering ordering = tsk_trait_comparable_compare(
        tsk_type_trait(tsk_array_view_const_element_type(array_view_1), TSK_TRAIT_COMPARABLE),
        tsk_array_view_const_get(array_view_1, i),
        tsk_array_view_const_get(array_view_2, i)
    );
    if (ordering != TSK_ORDERING_EQUAL) {
      return ordering;
    }
  }

  return (tsk_array_view_const_length(array_view_1) > tsk_array_view_const_length(array_view_2)) -
         (tsk_array_view_const_length(array_view_1) < tsk_array_view_const_length(array_view_2));
}
TskBoolean tsk_array_view_const_equals(TskArrayViewConst array_view_1, TskArrayViewConst array_view_2) {
  assert(tsk_array_view_const_is_valid(array_view_1));
  assert(tsk_array_view_const_is_valid(array_view_2));
  assert(tsk_array_view_const_element_type(array_view_1) == tsk_array_view_const_element_type(array_view_2));
  assert(tsk_type_has_trait(tsk_array_view_const_element_type(array_view_1), TSK_TRAIT_EQUATABLE));

  if (tsk_array_view_const_length(array_view_1) != tsk_array_view_const_length(array_view_2)) {
    return TSK_FALSE;
  }

  for (TskUSize i = 0; i < tsk_array_view_const_length(array_view_1); i++) {
    if (!tsk_trait_equatable_equals(
            tsk_type_trait(tsk_array_view_const_element_type(array_view_1), TSK_TRAIT_EQUATABLE),
            tsk_array_view_const_get(array_view_1, i),
            tsk_array_view_const_get(array_view_2, i)
        )) {
      return TSK_FALSE;
    }
  }

  return TSK_TRUE;
}
TskEmpty tsk_array_view_const_hash(TskArrayViewConst array_view, const TskTraitHasher *trait_hasher, TskAny *hasher) {
  assert(tsk_array_view_const_is_valid(array_view));
  assert(tsk_type_has_trait(tsk_array_view_const_element_type(array_view), TSK_TRAIT_HASHABLE));

  tsk_usize_hash(tsk_array_view_const_length(array_view), trait_hasher, hasher);
  for (TskUSize i = 0; i < tsk_array_view_const_length(array_view); i++) {
    tsk_trait_hashable_hash(
        tsk_type_trait(tsk_array_view_const_element_type(array_view), TSK_TRAIT_HASHABLE),
        tsk_array_view_const_get(array_view, i),
        trait_hasher,
        hasher
    );
  }
}

static const TskTraitComplete tsk_array_view_const_trait_complete = {
  .size      = sizeof(TskArrayViewConst),
  .alignment = alignof(TskArrayViewConst),
};

static const TskTraitDroppable tsk_array_view_const_trait_droppable = {
  .drop = TSK_NULL,
};

static const TskTraitClonable tsk_array_view_const_trait_clonable = {
  .trait_complete = &tsk_array_view_const_trait_complete,
  .clone          = TSK_NULL,
};

static TskOrdering tsk_array_view_const_trait_comparable_compare(const TskAny *array_view_1, const TskAny *array_view_2) {
  return tsk_array_view_const_compare(*(const TskArrayViewConst *)array_view_1, *(const TskArrayViewConst *)array_view_2);
}
static const TskTraitComparable tsk_array_view_const_trait_comparable = {
  .compare = tsk_array_view_const_trait_comparable_compare,
};

static TskBoolean tsk_array_view_const_type_equatable_equals(const TskAny *array_view_1, const TskAny *array_view_2) {
  return tsk_array_view_const_equals(*(const TskArrayViewConst *)array_view_1, *(const TskArrayViewConst *)array_view_2);
}
static const TskTraitEquatable tsk_array_view_const_trait_equatable = {
  .equals = tsk_array_view_const_type_equatable_equals,
};

static TskEmpty tsk_array_view_const_trait_hashable_hash(const TskAny *array_view, const TskTraitHasher *trait_hasher, TskAny *hasher) {
  tsk_array_view_const_hash(*(const TskArrayViewConst *)array_view, trait_hasher, hasher);
}
static const TskTraitHashable tsk_array_view_const_trait_hashable = {
  .hash = tsk_array_view_const_trait_hashable_hash,
};

const TskType tsk_array_view_const_type = {
  .name   = "TskArrayViewConst",
  .traits = {
      [TSK_TRAIT_COMPLETE]   = &tsk_array_view_const_trait_complete,
      [TSK_TRAIT_DROPPABLE]  = &tsk_array_view_const_trait_droppable,
      [TSK_TRAIT_CLONABLE]   = &tsk_array_view_const_trait_clonable,
      [TSK_TRAIT_COMPARABLE] = &tsk_array_view_const_trait_comparable,
      [TSK_TRAIT_EQUATABLE]  = &tsk_array_view_const_trait_equatable,
      [TSK_TRAIT_HASHABLE]   = &tsk_array_view_const_trait_hashable,
  },
};

TskArrayView tsk_array_to_view(TskArray *array) {
  assert(tsk_array_is_valid(array));

  TskArrayView array_view = {
    .element_type = array->element_type,
    .elements     = array->elements,
    .length       = array->length,
    .stride       = 1,
  };

  assert(tsk_array_view_is_valid(array_view));

  return array_view;
}
TskArrayViewConst tsk_array_to_view_const(const TskArray *array) {
  assert(tsk_array_is_valid(array));

  TskArrayViewConst array_view = {
    .element_type = array->element_type,
    .elements     = array->elements,
    .length       = array->length,
    .stride       = 1,
  };

  assert(tsk_array_view_const_is_valid(array_view));

  return array_view;
}

TskArrayViewConst tsk_array_view_to_const(TskArrayView array_view) {
  assert(tsk_array_view_is_valid(array_view));

  TskArrayViewConst array_view_const = {
    .element_type = array_view.element_type,
    .elements     = array_view.elements,
    .length       = array_view.length,
    .stride       = array_view.stride,
  };

  assert(tsk_array_view_const_is_valid(array_view_const));

  return array_view_const;
} */
