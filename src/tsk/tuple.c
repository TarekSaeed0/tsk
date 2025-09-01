#include <tsk/tuple.h>

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
#include <stdio.h>
#include <string.h>

typedef struct TskTupleType TskTupleType;
struct TskTupleType {
	TskType                tuple_type;
	TskCharacter           tuple_type_name[96];
	TskTraitComplete       tuple_type_complete_trait;
	TskTypeTraitTable      tuple_type_trait_table;
	TskTypeTraitTableEntry tuple_type_trait_table_entries[16];
	const TskType *const  *element_types;
	const TskUSize        *element_offsets;
	TskUSize               length;
};

TskEmpty tsk_tuple_drop(const TskType *tuple_type, TskTuple *tuple) {
	assert(tsk_tuple_type_is_valid(tuple_type));
	assert(tuple != TSK_NULL);

	for (TskUSize i = 0; i < tsk_tuple_length(tuple_type); i++) {
		assert(tsk_type_has_trait(tsk_tuple_element_type(tuple_type, i), TSK_TRAIT_ID_DROPPABLE));

		tsk_trait_droppable_drop(
		    tsk_tuple_element_type(tuple_type, i),
		    tsk_tuple_get(tuple_type, tuple, i)
		);
	}
}
TskBoolean tsk_tuple_clone(const TskType *tuple_type, const TskTuple *tuple_1, TskTuple *tuple_2) {
	assert(tsk_tuple_type_is_valid(tuple_type));
	assert(tuple_1 != TSK_NULL);
	assert(tuple_2 != TSK_NULL);

	for (TskUSize i = 0; i < tsk_tuple_length(tuple_type); i++) {
		assert(tsk_type_has_trait(tsk_tuple_element_type(tuple_type, i), TSK_TRAIT_ID_CLONABLE));

		if (!tsk_trait_clonable_clone(
		        tsk_tuple_element_type(tuple_type, i),
		        tsk_tuple_get_const(tuple_type, tuple_1, i),
		        tsk_tuple_get(tuple_type, tuple_2, i)
		    )) {
			for (TskUSize j = 0; j < i; j++) {
				assert(tsk_type_has_trait(tsk_tuple_element_type(tuple_type, j), TSK_TRAIT_ID_DROPPABLE));

				tsk_trait_droppable_drop(
				    tsk_tuple_element_type(tuple_type, j),
				    tsk_tuple_get(tuple_type, tuple_2, j)
				);
			}
			return TSK_FALSE;
		}
	}

	return TSK_TRUE;
}
const TskType *tsk_tuple_element_type(const TskType *tuple_type, TskUSize index) {
	assert(tsk_tuple_type_is_valid(tuple_type));
	assert(index < tsk_tuple_length(tuple_type));

	return ((const TskTupleType *)tuple_type)->element_types[index];
}
TskUSize tsk_tuple_length(const TskType *tuple_type) {
	assert(tsk_tuple_type_is_valid(tuple_type));

	return ((const TskTupleType *)tuple_type)->length;
}
TskAny *tsk_tuple_get(const TskType *tuple_type, TskTuple *tuple, TskUSize index) {
	assert(tsk_tuple_type_is_valid(tuple_type));
	assert(tuple != TSK_NULL);
	assert(index < tsk_tuple_length(tuple_type));

	return (TskU8 *)tuple + ((const TskTupleType *)tuple_type)->element_offsets[index];
}
const TskAny *tsk_tuple_get_const(const TskType *tuple_type, const TskTuple *tuple, TskUSize index) {
	assert(tsk_tuple_type_is_valid(tuple_type));
	assert(tuple != TSK_NULL);
	assert(index < tsk_tuple_length(tuple_type));

	return (const TskU8 *)tuple + ((const TskTupleType *)tuple_type)->element_offsets[index];
}
TskOrdering tsk_tuple_compare(const TskType *tuple_type, const TskTuple *tuple_1, const TskTuple *tuple_2) {
	assert(tsk_tuple_type_is_valid(tuple_type));
	assert(tuple_1 != TSK_NULL);
	assert(tuple_2 != TSK_NULL);

	for (TskUSize i = 0; i < tsk_tuple_length(tuple_type); i++) {
		assert(tsk_type_has_trait(tsk_tuple_element_type(tuple_type, i), TSK_TRAIT_ID_COMPARABLE));

		TskOrdering ordering = tsk_trait_comparable_compare(
		    tsk_tuple_element_type(tuple_type, i),
		    tsk_tuple_get_const(tuple_type, tuple_1, i),
		    tsk_tuple_get_const(tuple_type, tuple_2, i)
		);
		if (ordering != TSK_ORDERING_EQUAL) {
			return ordering;
		}
	}

	return TSK_ORDERING_EQUAL;
}
TskBoolean tsk_tuple_equals(const TskType *tuple_type, const TskTuple *tuple_1, const TskTuple *tuple_2) {
	assert(tsk_tuple_type_is_valid(tuple_type));
	assert(tuple_1 != TSK_NULL);
	assert(tuple_2 != TSK_NULL);

	for (TskUSize i = 0; i < tsk_tuple_length(tuple_type); i++) {
		assert(tsk_type_has_trait(tsk_tuple_element_type(tuple_type, i), TSK_TRAIT_ID_EQUATABLE));

		if (!tsk_trait_equatable_equals(
		        tsk_tuple_element_type(tuple_type, i),
		        tsk_tuple_get_const(tuple_type, tuple_1, i),
		        tsk_tuple_get_const(tuple_type, tuple_2, i)
		    )) {
			return TSK_FALSE;
		}
	}

	return TSK_TRUE;
}
TskEmpty tsk_tuple_hash(const TskType *tuple_type, const TskTuple *tuple, const TskType *hasher_type, TskAny *hasher) {
	assert(tsk_tuple_type_is_valid(tuple_type));
	assert(tuple != TSK_NULL);
	assert(tsk_type_is_valid(hasher_type));
	assert(tsk_type_has_trait(hasher_type, TSK_TRAIT_ID_HASHER));
	assert(hasher != TSK_NULL);

	for (TskUSize i = 0; i < tsk_tuple_length(tuple_type); i++) {
		assert(tsk_type_has_trait(tsk_tuple_element_type(tuple_type, i), TSK_TRAIT_ID_HASHABLE));

		tsk_trait_hashable_hash(
		    tsk_tuple_element_type(tuple_type, i),
		    tsk_tuple_get_const(tuple_type, tuple, i),
		    hasher_type,
		    hasher
		);
	}
}

TskEmpty tsk_tuple_type_trait_droppable_drop(const TskType *droppable_type, TskAny *droppable) {
	tsk_tuple_drop(droppable_type, droppable);
}
TskBoolean tsk_tuple_type_trait_clonable_clone(const TskType *clonable_type, const TskAny *clonable_1, TskAny *clonable_2) {
	return tsk_tuple_clone(clonable_type, clonable_1, clonable_2);
}
TskOrdering tsk_tuple_type_trait_comparable_compare(const TskType *comparable_type, const TskAny *comparable_1, const TskAny *comparable_2) {
	return tsk_tuple_compare(comparable_type, comparable_1, comparable_2);
}
TskBoolean tsk_tuple_type_trait_equatable_equals(const TskType *equatable_type, const TskAny *equatable_1, const TskAny *equatable_2) {
	return tsk_tuple_equals(equatable_type, equatable_1, equatable_2);
}
TskEmpty tsk_tuple_type_trait_hashable_hash(const TskType *hashable_type, const TskAny *hashable, const TskType *hasher_type, TskAny *hasher) {
	tsk_tuple_hash(hashable_type, hashable, hasher_type, hasher);
}

const TskTraitDroppable tsk_tuple_type_trait_droppable = {
	.drop = tsk_tuple_type_trait_droppable_drop,
};
const TskTraitClonable tsk_tuple_type_trait_clonable = {
	.clone = tsk_tuple_type_trait_clonable_clone,
};
const TskTraitComparable tsk_tuple_type_trait_comparable = {
	.compare = tsk_tuple_type_trait_comparable_compare,
};
const TskTraitEquatable tsk_tuple_type_trait_equatable = {
	.equals = tsk_tuple_type_trait_equatable_equals,
};
const TskTraitHashable tsk_tuple_type_trait_hashable = {
	.hash = tsk_tuple_type_trait_hashable_hash,
};

#define TSK_TUPLE_TYPES_CAPACITY ((TskUSize)1 << 7)
#define TSK_TUPLE_TYPES_ELEMENTS_CAPACITY ((TskUSize)1 << 9)

TskTupleType   tsk_tuple_types[TSK_TUPLE_TYPES_CAPACITY];
const TskType *tsk_tuple_types_element_types[TSK_TUPLE_TYPES_ELEMENTS_CAPACITY];
TskUSize       tsk_tuple_types_element_offsets[TSK_TUPLE_TYPES_ELEMENTS_CAPACITY];
TskUSize       tsk_tuple_types_length = 0;

TskBoolean tsk_tuple_type_is_valid(const TskType *tuple_type) {
	return tsk_type_is_valid(tuple_type) &&
	       &tsk_tuple_types[0] <= (const TskTupleType *)tuple_type && (const TskTupleType *)tuple_type < &tsk_tuple_types[TSK_TUPLE_TYPES_CAPACITY];
}
const TskType *tsk_tuple_type(const TskType **element_types, TskUSize length) {
	if (tsk_tuple_types_length + length > TSK_TUPLE_TYPES_ELEMENTS_CAPACITY) {
		return TSK_NULL;
	}

	const TskType             *hasher_type = tsk_trait_builder_built_type(tsk_default_hasher_builder_type);
	alignas(max_align_t) TskU8 hasher[tsk_trait_complete_size(hasher_type)];
	tsk_trait_builder_build(tsk_default_hasher_builder_type, tsk_default_hasher_builder, hasher);

	for (TskUSize i = 0; i < length; i++) {
		assert(tsk_type_is_valid(element_types[i]));
		tsk_trait_hasher_combine(hasher_type, hasher, (const TskU8 *)&element_types[i], sizeof(element_types[i])); // NOLINT(bugprone-sizeof-expression)
	}
	TskU64 hash = tsk_trait_hasher_finalize(hasher_type, hasher);

	tsk_trait_droppable_drop(hasher_type, hasher);

	TskUSize starting_index = hash & (TSK_TUPLE_TYPES_CAPACITY - 1);
	TskUSize index          = starting_index;
	while (tsk_tuple_types[index].element_types != TSK_NULL) {
		TskBoolean is_equal = true;
		if (tsk_tuple_types[index].length == length) {
			for (TskUSize i = 0; i < length; i++) {
				if (tsk_tuple_types[index].element_types[i] != element_types[i]) {
					is_equal = false;
					break;
				}
			}
		} else {
			is_equal = false;
		}
		if (is_equal) {
			return &tsk_tuple_types[index].tuple_type;
		}
		index = (index + 1) & (TSK_TUPLE_TYPES_CAPACITY - 1);
		if (index == starting_index) {
			return TSK_NULL;
		}
	}

	TskUSize *element_offsets = &tsk_tuple_types_element_offsets[tsk_tuple_types_length];
	TskUSize  element_offset  = 0;
	TskUSize  alignment       = 1;
	for (TskUSize i = 0; i < length; i++) {
		assert(tsk_type_is_valid(element_types[i]));
		assert(tsk_type_has_trait(element_types[i], TSK_TRAIT_ID_COMPLETE));

		TskUSize element_size      = tsk_trait_complete_size(element_types[i]);
		TskUSize element_alignment = tsk_trait_complete_alignment(element_types[i]);

		if (element_alignment > alignment) {
			alignment = element_alignment;
		}

		element_offset     = (element_offset + element_alignment - 1) & ~(element_alignment - 1);
		element_offsets[i] = element_offset;
		element_offset += element_size;
	}

	TskUSize size                                          = (element_offset + alignment - 1) & ~(alignment - 1);

	tsk_tuple_types[index].tuple_type.trait_table          = &tsk_tuple_types[index].tuple_type_trait_table;
	tsk_tuple_types[index].tuple_type_trait_table.entries  = tsk_tuple_types[index].tuple_type_trait_table_entries;
	tsk_tuple_types[index].tuple_type_trait_table.capacity = sizeof(tsk_tuple_types[index].tuple_type_trait_table_entries) / sizeof(tsk_tuple_types[index].tuple_type_trait_table_entries[0]);

	tsk_tuple_types[index].tuple_type_complete_trait       = (TskTraitComplete){
		      .size      = size,
		      .alignment = alignment,
	};

	TskBoolean is_droppable  = true;
	TskBoolean is_clonable   = true;
	TskBoolean is_comparable = true;
	TskBoolean is_equatable  = true;
	TskBoolean is_hashable   = true;
	for (TskUSize i = 0; i < length; i++) {
		assert(tsk_type_is_valid(element_types[i]));

		if (!tsk_type_has_trait(element_types[i], TSK_TRAIT_ID_DROPPABLE)) {
			is_droppable = false;
		}
		if (!tsk_type_has_trait(element_types[i], TSK_TRAIT_ID_CLONABLE)) {
			is_clonable = false;
		}
		if (!tsk_type_has_trait(element_types[i], TSK_TRAIT_ID_COMPARABLE)) {
			is_comparable = false;
		}
		if (!tsk_type_has_trait(element_types[i], TSK_TRAIT_ID_EQUATABLE)) {
			is_equatable = false;
		}
		if (!tsk_type_has_trait(element_types[i], TSK_TRAIT_ID_HASHABLE)) {
			is_hashable = false;
		}
	}

	tsk_tuple_types[index].tuple_type_trait_table.entries[TSK_TRAIT_ID_COMPLETE & (tsk_tuple_types[index].tuple_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
		.trait_id   = TSK_TRAIT_ID_COMPLETE,
		.trait_data = &tsk_tuple_types[index].tuple_type_complete_trait,
	};
	if (is_droppable) {
		tsk_tuple_types[index].tuple_type_trait_table.entries[TSK_TRAIT_ID_DROPPABLE & (tsk_tuple_types[index].tuple_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_DROPPABLE,
			.trait_data = &tsk_tuple_type_trait_droppable,
		};
	}
	if (is_clonable) {
		tsk_tuple_types[index].tuple_type_trait_table.entries[TSK_TRAIT_ID_CLONABLE & (tsk_tuple_types[index].tuple_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_CLONABLE,
			.trait_data = &tsk_tuple_type_trait_clonable,
		};
	}
	if (is_comparable) {
		tsk_tuple_types[index].tuple_type_trait_table.entries[TSK_TRAIT_ID_COMPARABLE & (tsk_tuple_types[index].tuple_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_COMPARABLE,
			.trait_data = &tsk_tuple_type_trait_comparable,
		};
	}
	if (is_equatable) {
		tsk_tuple_types[index].tuple_type_trait_table.entries[TSK_TRAIT_ID_EQUATABLE & (tsk_tuple_types[index].tuple_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_EQUATABLE,
			.trait_data = &tsk_tuple_type_trait_equatable,
		};
	}
	if (is_hashable) {
		tsk_tuple_types[index].tuple_type_trait_table.entries[TSK_TRAIT_ID_HASHABLE & (tsk_tuple_types[index].tuple_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_HASHABLE,
			.trait_data = &tsk_tuple_type_trait_hashable,
		};
	}

	tsk_tuple_types[index].element_types   = memcpy(&tsk_tuple_types_element_types[tsk_tuple_types_length], element_types, length * sizeof(const TskType *));
	tsk_tuple_types[index].element_offsets = element_offsets;
	tsk_tuple_types_length += length;

	tsk_tuple_types[index].length = length;

	TskUSize j                    = (TskUSize)snprintf(
      tsk_tuple_types[index].tuple_type_name,
      sizeof(tsk_tuple_types[index].tuple_type_name),
      "TskTuple<"
  );
	for (TskUSize i = 0; i < length && j < sizeof(tsk_tuple_types[index].tuple_type_name); i++) {
		j += (TskUSize)snprintf(
		    tsk_tuple_types[index].tuple_type_name + j,
		    sizeof(tsk_tuple_types[index].tuple_type_name) - j,
		    i != 0 ? ", %s" : "%s",
		    tsk_type_name(element_types[i])
		);
	}
	(TskEmpty) snprintf(
	    tsk_tuple_types[index].tuple_type_name + j,
	    sizeof(tsk_tuple_types[index].tuple_type_name) - j,
	    ">"
	);
	tsk_tuple_types[index].tuple_type.name = tsk_tuple_types[index].tuple_type_name;

	const TskType *tuple_type              = &tsk_tuple_types[index].tuple_type;

	assert(tsk_tuple_type_is_valid(tuple_type));

	return tuple_type;
}
