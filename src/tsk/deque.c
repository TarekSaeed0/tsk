#include <tsk/deque.h>

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

// TODO: allow zero-sized types in deque

// PERF: consider making the segment size dependent on the element size

#define TSK_DEQUE_SEGMENT_SIZE ((TskUSize)1 << 4)

typedef struct TskDequeType TskDequeType;
struct TskDequeType {
	TskType                deque_type;
	TskCharacter           deque_type_name[40];
	TskTypeTraitTable      deque_type_trait_table;
	TskTypeTraitTableEntry deque_type_trait_table_entries[16];
	const TskType         *element_type;
};

TskBoolean tsk_deque_is_valid(const TskType *deque_type, const TskDeque *deque) {
	assert(tsk_deque_type_is_valid(deque_type));

	return deque != TSK_NULL && (deque->segments != TSK_NULL || deque->segments_length == 0) &&
	       deque->front_index <= deque->back_index &&
	       (deque->back_index - deque->front_index <= deque->segments_length * TSK_DEQUE_SEGMENT_SIZE);
}
TskDeque tsk_deque_new(const TskType *deque_type) {
	assert(tsk_deque_type_is_valid(deque_type));

	TskDeque deque = {
		.segments        = TSK_NULL,
		.segments_length = 0,
		.front_index     = 0,
		.back_index      = 0,
	};

	assert(tsk_deque_is_valid(deque_type, &deque));

	return deque;
}
TskEmpty tsk_deque_drop(const TskType *deque_type, TskDeque *deque) {
	assert(tsk_deque_type_is_valid(deque_type));
	assert(tsk_deque_is_valid(deque_type, deque));

	tsk_deque_clear(deque_type, deque);

	for (TskUSize i = 0; i < tsk_deque_capacity(deque_type, deque) / TSK_DEQUE_SEGMENT_SIZE; i++) {
		free(deque->segments[i]);
	}
	free(deque->segments);

	deque->segments        = TSK_NULL;
	deque->segments_length = 0;
}
TskBoolean tsk_deque_clone(const TskType *deque_type, const TskDeque *deque_1, TskDeque *deque_2) {
	assert(tsk_deque_type_is_valid(deque_type));
	assert(tsk_deque_is_valid(deque_type, deque_1));
	assert(deque_2 != TSK_NULL);

	if (tsk_deque_is_empty(deque_type, deque_1)) {
		*deque_2 = tsk_deque_new(deque_type);
		return TSK_TRUE;
	}

	TskUSize segments_length = (tsk_deque_length(deque_type, deque_1) + TSK_DEQUE_SEGMENT_SIZE - 1) / TSK_DEQUE_SEGMENT_SIZE;
	TskAny **segments        = malloc(segments_length * sizeof(*segments));
	if (segments == TSK_NULL) {
		return TSK_FALSE;
	}

	for (TskUSize i = 0; i < segments_length; i++) {
		segments[i] = malloc(TSK_DEQUE_SEGMENT_SIZE * tsk_trait_complete_size(tsk_deque_element_type(deque_type)));
		if (segments[i] == TSK_NULL) {
			goto cleanup;
		}
	}

	for (TskUSize i = 0; i < tsk_deque_length(deque_type, deque_1); i++) {
		if (!tsk_trait_clonable_clone(
		        tsk_deque_element_type(deque_type),
		        tsk_deque_get_const(deque_type, deque_1, i),
		        tsk_deque_get(deque_type, deque_2, i)
		    )) {
			for (TskUSize j = 0; j < i; j++) {
				tsk_trait_droppable_drop(
				    tsk_deque_element_type(deque_type),
				    tsk_deque_get(deque_type, deque_2, i)
				);
			}
			goto cleanup;
		}
	}

	deque_2->segments        = segments;
	deque_2->segments_length = segments_length;
	deque_2->front_index     = 0;
	deque_2->back_index      = tsk_deque_length(deque_type, deque_1);

	return TSK_FALSE;
cleanup:
	for (TskUSize i = 0; i < segments_length; i++) {
		free(segments[i]);
	}
	free(segments);
	return TSK_FALSE;
}
const TskType *tsk_deque_element_type(const TskType *deque_type) {
	assert(tsk_deque_type_is_valid(deque_type));

	return ((const TskDequeType *)deque_type)->element_type;
}
TskUSize tsk_deque_length(const TskType *deque_type, const TskDeque *deque) {
	assert(tsk_deque_type_is_valid(deque_type));
	assert(tsk_deque_is_valid(deque_type, deque));

	return deque->back_index - deque->front_index;
}
TskBoolean tsk_deque_is_empty(const TskType *deque_type, const TskDeque *deque) {
	assert(tsk_deque_type_is_valid(deque_type));
	assert(tsk_deque_is_valid(deque_type, deque));

	return tsk_deque_length(deque_type, deque) == 0;
}
TskUSize tsk_deque_capacity(const TskType *deque_type, const TskDeque *deque) {
	assert(tsk_deque_type_is_valid(deque_type));
	assert(tsk_deque_is_valid(deque_type, deque));

	return deque->segments_length * TSK_DEQUE_SEGMENT_SIZE;
}
TskAny *tsk_deque_get(const TskType *deque_type, TskDeque *deque, TskUSize index) {
	assert(tsk_deque_type_is_valid(deque_type));
	assert(tsk_deque_is_valid(deque_type, deque));
	assert(index < tsk_deque_length(deque_type, deque));

	return (TskU8 *)deque->segments[(deque->front_index + index) / TSK_DEQUE_SEGMENT_SIZE] + (((deque->front_index + index) % TSK_DEQUE_SEGMENT_SIZE) * tsk_trait_complete_size(tsk_deque_element_type(deque_type)));
}
const TskAny *tsk_deque_get_const(const TskType *deque_type, const TskDeque *deque, TskUSize index) {
	assert(tsk_deque_type_is_valid(deque_type));
	assert(tsk_deque_is_valid(deque_type, deque));
	assert(index < tsk_deque_length(deque_type, deque));

	return (const TskU8 *)deque->segments[(deque->front_index + index) / TSK_DEQUE_SEGMENT_SIZE] + (((deque->front_index + index) % TSK_DEQUE_SEGMENT_SIZE) * tsk_trait_complete_size(tsk_deque_element_type(deque_type)));
}
TskAny *tsk_deque_front(const TskType *deque_type, TskDeque *deque) {
	assert(tsk_deque_type_is_valid(deque_type));
	assert(tsk_deque_is_valid(deque_type, deque));

	if (tsk_deque_is_empty(deque_type, deque)) {
		return TSK_NULL;
	}

	return tsk_deque_get(deque_type, deque, 0);
}
const TskAny *tsk_deque_front_const(const TskType *deque_type, const TskDeque *deque) {
	assert(tsk_deque_type_is_valid(deque_type));
	assert(tsk_deque_is_valid(deque_type, deque));

	if (tsk_deque_is_empty(deque_type, deque)) {
		return TSK_NULL;
	}

	return tsk_deque_get_const(deque_type, deque, 0);
}
TskAny *tsk_deque_back(const TskType *deque_type, TskDeque *deque) {
	assert(tsk_deque_type_is_valid(deque_type));
	assert(tsk_deque_is_valid(deque_type, deque));

	if (tsk_deque_is_empty(deque_type, deque)) {
		return TSK_NULL;
	}

	return tsk_deque_get(deque_type, deque, tsk_deque_length(deque_type, deque) - 1);
}
const TskAny *tsk_deque_back_const(const TskType *deque_type, const TskDeque *deque) {
	assert(tsk_deque_type_is_valid(deque_type));
	assert(tsk_deque_is_valid(deque_type, deque));

	if (tsk_deque_is_empty(deque_type, deque)) {
		return TSK_NULL;
	}

	return tsk_deque_get_const(deque_type, deque, tsk_deque_length(deque_type, deque) - 1);
}
TskEmpty tsk_deque_clear(const TskType *deque_type, TskDeque *deque) {
	assert(tsk_deque_type_is_valid(deque_type));
	assert(tsk_deque_is_valid(deque_type, deque));
	assert(tsk_type_has_trait(tsk_deque_element_type(deque_type), TSK_TRAIT_ID_DROPPABLE));

	for (TskUSize i = 0; i < tsk_deque_length(deque_type, deque); i++) {
		tsk_trait_droppable_drop(
		    tsk_deque_element_type(deque_type),
		    tsk_deque_get(deque_type, deque, i)
		);
	}

	deque->front_index = deque->back_index = 0;
}
TskBoolean tsk_deque_push_front(const TskType *deque_type, TskDeque *deque, TskAny *element) {
	assert(tsk_deque_type_is_valid(deque_type));
	assert(tsk_deque_is_valid(deque_type, deque));
	assert(element != TSK_NULL);

	if (deque->front_index == 0) {
		TskAny **segments = malloc((deque->segments_length + 1) * sizeof(*segments));
		if (segments == TSK_NULL) {
			return TSK_FALSE;
		}

		segments[0] = malloc(TSK_DEQUE_SEGMENT_SIZE * tsk_trait_complete_size(tsk_deque_element_type(deque_type)));
		if (segments[0] == TSK_NULL) {
			free(segments);
			return TSK_FALSE;
		}

		if (deque->segments != TSK_NULL) {
			memcpy(segments + 1, deque->segments, deque->segments_length * sizeof(*segments));
		}

		free(deque->segments);

		deque->segments = segments;
		deque->segments_length++;

		deque->front_index += TSK_DEQUE_SEGMENT_SIZE;
		deque->back_index += TSK_DEQUE_SEGMENT_SIZE;
	}

	deque->front_index--;

	memcpy(
	    tsk_deque_front(deque_type, deque),
	    element,
	    tsk_trait_complete_size(tsk_deque_element_type(deque_type))
	);

	return TSK_TRUE;
}
TskBoolean tsk_deque_push_back(const TskType *deque_type, TskDeque *deque, TskAny *element) {
	assert(tsk_deque_type_is_valid(deque_type));
	assert(tsk_deque_is_valid(deque_type, deque));
	assert(element != TSK_NULL);

	if (deque->back_index == tsk_deque_capacity(deque_type, deque)) {
		TskAny **segments = malloc((deque->segments_length + 1) * sizeof(*segments));
		if (segments == TSK_NULL) {
			return TSK_FALSE;
		}

		segments[deque->segments_length] = malloc(TSK_DEQUE_SEGMENT_SIZE * tsk_trait_complete_size(tsk_deque_element_type(deque_type)));
		if (segments[deque->segments_length] == TSK_NULL) {
			free(segments);
			return TSK_FALSE;
		}

		if (deque->segments != TSK_NULL) {
			memcpy(segments, deque->segments, deque->segments_length * sizeof(*segments));
		}

		free(deque->segments);

		deque->segments = segments;
		deque->segments_length++;
	}

	memcpy(
	    tsk_deque_back(deque_type, deque),
	    element,
	    tsk_trait_complete_size(tsk_deque_element_type(deque_type))
	);

	deque->back_index++;

	return TSK_TRUE;
}
TskBoolean tsk_deque_pop_back(const TskType *deque_type, TskDeque *deque, TskAny *element) {
	assert(tsk_deque_type_is_valid(deque_type));
	assert(tsk_deque_is_valid(deque_type, deque));

	if (tsk_deque_is_empty(deque_type, deque)) {
		return TSK_FALSE;
	}

	if (element != TSK_NULL) {
		memcpy(
		    element,
		    tsk_deque_get_const(deque_type, deque, tsk_deque_length(deque_type, deque) - 1),
		    tsk_trait_complete_size(tsk_deque_element_type(deque_type))
		);
	} else {
		assert(tsk_type_has_trait(tsk_deque_element_type(deque_type), TSK_TRAIT_ID_DROPPABLE));
		tsk_trait_droppable_drop(
		    tsk_deque_element_type(deque_type),
		    tsk_deque_get(deque_type, deque, tsk_deque_length(deque_type, deque) - 1)
		);
	}

	deque->back_index--;

	return TSK_TRUE;
}
TskOrdering tsk_deque_compare(const TskType *deque_type, const TskDeque *deque_1, const TskDeque *deque_2) {
	assert(tsk_deque_type_is_valid(deque_type));
	assert(tsk_deque_is_valid(deque_type, deque_1));
	assert(tsk_deque_is_valid(deque_type, deque_2));
	assert(tsk_type_has_trait(tsk_deque_element_type(deque_type), TSK_TRAIT_ID_COMPARABLE));

	for (TskUSize i = 0; i < tsk_deque_length(deque_type, deque_1) && i < tsk_deque_length(deque_type, deque_2); i++) {
		TskOrdering ordering = tsk_trait_comparable_compare(
		    tsk_deque_element_type(deque_type),
		    tsk_deque_get_const(deque_type, deque_1, i),
		    tsk_deque_get_const(deque_type, deque_2, i)
		);
		if (ordering != TSK_ORDERING_EQUAL) {
			return ordering;
		}
	}

	return (tsk_deque_length(deque_type, deque_1) > tsk_deque_length(deque_type, deque_2)) -
	       (tsk_deque_length(deque_type, deque_1) < tsk_deque_length(deque_type, deque_2));
}
TskBoolean tsk_deque_equals(const TskType *deque_type, const TskDeque *deque_1, const TskDeque *deque_2) {
	assert(tsk_deque_type_is_valid(deque_type));
	assert(tsk_deque_is_valid(deque_type, deque_1));
	assert(tsk_deque_is_valid(deque_type, deque_2));
	assert(tsk_type_has_trait(tsk_deque_element_type(deque_type), TSK_TRAIT_ID_EQUATABLE));

	if (tsk_deque_length(deque_type, deque_1) != tsk_deque_length(deque_type, deque_2)) {
		return TSK_FALSE;
	}

	for (TskUSize i = 0; i < tsk_deque_length(deque_type, deque_1); i++) {
		if (!tsk_trait_equatable_equals(
		        tsk_deque_element_type(deque_type),
		        tsk_deque_get_const(deque_type, deque_1, i),
		        tsk_deque_get_const(deque_type, deque_2, i)
		    )) {
			return TSK_FALSE;
		}
	}

	return TSK_TRUE;
}
TskEmpty tsk_deque_hash(const TskType *deque_type, const TskDeque *deque, const TskType *hasher_type, TskAny *hasher) {
	assert(tsk_deque_type_is_valid(deque_type));
	assert(tsk_deque_is_valid(deque_type, deque));
	assert(tsk_type_has_trait(tsk_deque_element_type(deque_type), TSK_TRAIT_ID_HASHABLE));

	TskUSize length = tsk_deque_length(deque_type, deque);
	tsk_trait_hasher_combine(
	    hasher_type,
	    hasher,
	    (const TskU8 *)&length,
	    sizeof(length)
	);

	for (TskUSize i = 0; i < tsk_deque_length(deque_type, deque); i++) {
		tsk_trait_hashable_hash(
		    tsk_deque_element_type(deque_type),
		    tsk_deque_get_const(deque_type, deque, i),
		    hasher_type,
		    hasher
		);
	}
}

TskEmpty tsk_deque_type_trait_droppable_drop(const TskType *droppable_type, TskAny *droppable) {
	tsk_deque_drop(droppable_type, droppable);
}
TskBoolean tsk_deque_type_trait_clonable_clone(const TskType *clonable_type, const TskAny *clonable_1, TskAny *clonable_2) {
	return tsk_deque_clone(clonable_type, clonable_1, clonable_2);
}
TskOrdering tsk_deque_type_trait_comparable_compare(const TskType *comparable_type, const TskAny *comparable_1, const TskAny *comparable_2) {
	return tsk_deque_compare(comparable_type, comparable_1, comparable_2);
}
TskBoolean tsk_deque_type_trait_equatable_equals(const TskType *equatable_type, const TskAny *equatable_1, const TskAny *equatable_2) {
	return tsk_deque_equals(equatable_type, equatable_1, equatable_2);
}
TskEmpty tsk_deque_type_trait_hashable_hash(const TskType *hashable_type, const TskAny *hashable, const TskType *hasher_type, TskAny *hasher) {
	tsk_deque_hash(hashable_type, hashable, hasher_type, hasher);
}

const TskTraitComplete tsk_deque_type_trait_complete = {
	.size      = sizeof(TskDeque),
	.alignment = alignof(TskDeque),
};
const TskTraitDroppable tsk_deque_type_trait_droppable = {
	.drop = tsk_deque_type_trait_droppable_drop,
};
const TskTraitClonable tsk_deque_type_trait_clonable = {
	.clone = tsk_deque_type_trait_clonable_clone,
};
const TskTraitComparable tsk_deque_type_trait_comparable = {
	.compare = tsk_deque_type_trait_comparable_compare,
};
const TskTraitEquatable tsk_deque_type_trait_equatable = {
	.equals = tsk_deque_type_trait_equatable_equals,
};
const TskTraitHashable tsk_deque_type_trait_hashable = {
	.hash = tsk_deque_type_trait_hashable_hash,
};

#define TSK_DEQUE_TYPES_CAPACITY ((TskUSize)1 << 7)

TskDequeType tsk_deque_types[TSK_DEQUE_TYPES_CAPACITY];

TskBoolean tsk_deque_type_is_valid(const TskType *deque_type) {
	return tsk_type_is_valid(deque_type) &&
	       &tsk_deque_types[0] <= (const TskDequeType *)deque_type && (const TskDequeType *)deque_type < &tsk_deque_types[TSK_DEQUE_TYPES_CAPACITY];
}
const TskType *tsk_deque_type(const TskType *element_type) {
	assert(tsk_type_is_valid(element_type));
	assert(tsk_type_has_trait(element_type, TSK_TRAIT_ID_COMPLETE));

	const TskType             *hasher_type = tsk_trait_builder_built_type(tsk_default_hasher_builder_type);
	alignas(max_align_t) TskU8 hasher[tsk_trait_complete_size(hasher_type)];
	tsk_trait_builder_build(tsk_default_hasher_builder_type, tsk_default_hasher_builder, hasher);

	tsk_trait_hasher_combine(hasher_type, hasher, (const TskU8 *)&element_type, sizeof(element_type)); // NOLINT(bugprone-sizeof-expression)
	TskU64 hash = tsk_trait_hasher_finalize(hasher_type, hasher);

	tsk_trait_droppable_drop(hasher_type, hasher);

	TskUSize starting_index = hash & (TSK_DEQUE_TYPES_CAPACITY - 1);
	TskUSize index          = starting_index;
	while (tsk_deque_types[index].element_type != TSK_NULL) {
		if (tsk_deque_types[index].element_type == element_type) {
			return &tsk_deque_types[index].deque_type;
		}
		index = (index + 1) & (TSK_DEQUE_TYPES_CAPACITY - 1);
		if (index == starting_index) {
			return TSK_NULL;
		}
	}

	tsk_deque_types[index].deque_type.trait_table                                                                                               = &tsk_deque_types[index].deque_type_trait_table;
	tsk_deque_types[index].deque_type_trait_table.entries                                                                                       = tsk_deque_types[index].deque_type_trait_table_entries;
	tsk_deque_types[index].deque_type_trait_table.capacity                                                                                      = sizeof(tsk_deque_types[index].deque_type_trait_table_entries) / sizeof(tsk_deque_types[index].deque_type_trait_table_entries[0]);

	tsk_deque_types[index].deque_type_trait_table.entries[TSK_TRAIT_ID_COMPLETE & (tsk_deque_types[index].deque_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
		.trait_id   = TSK_TRAIT_ID_COMPLETE,
		.trait_data = &tsk_deque_type_trait_complete,
	};
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_DROPPABLE)) {
		tsk_deque_types[index].deque_type_trait_table.entries[TSK_TRAIT_ID_DROPPABLE & (tsk_deque_types[index].deque_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_DROPPABLE,
			.trait_data = &tsk_deque_type_trait_droppable,
		};
	}
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_CLONABLE)) {
		tsk_deque_types[index].deque_type_trait_table.entries[TSK_TRAIT_ID_CLONABLE & (tsk_deque_types[index].deque_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_CLONABLE,
			.trait_data = &tsk_deque_type_trait_clonable,
		};
	}
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_COMPARABLE)) {
		tsk_deque_types[index].deque_type_trait_table.entries[TSK_TRAIT_ID_COMPARABLE & (tsk_deque_types[index].deque_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_COMPARABLE,
			.trait_data = &tsk_deque_type_trait_comparable,
		};
	}
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_EQUATABLE)) {
		tsk_deque_types[index].deque_type_trait_table.entries[TSK_TRAIT_ID_EQUATABLE & (tsk_deque_types[index].deque_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_EQUATABLE,
			.trait_data = &tsk_deque_type_trait_equatable,
		};
	}
	if (tsk_type_has_trait(element_type, TSK_TRAIT_ID_HASHABLE)) {
		tsk_deque_types[index].deque_type_trait_table.entries[TSK_TRAIT_ID_HASHABLE & (tsk_deque_types[index].deque_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_HASHABLE,
			.trait_data = &tsk_deque_type_trait_hashable,
		};
	}

	tsk_deque_types[index].element_type = element_type;

	(void)snprintf(
	    tsk_deque_types[index].deque_type_name,
	    sizeof(tsk_deque_types[index].deque_type_name),
	    "TskDeque<%s>",
	    tsk_type_name(element_type)
	);
	tsk_deque_types[index].deque_type.name = tsk_deque_types[index].deque_type_name;

	const TskType *deque_type              = &tsk_deque_types[index].deque_type;

	assert(tsk_deque_type_is_valid(deque_type));

	return deque_type;
}
