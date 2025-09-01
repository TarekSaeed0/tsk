#include <tsk/reference.h>

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

typedef struct TskReferenceType TskReferenceType;
struct TskReferenceType {
	TskType        reference_type;
	TskCharacter   reference_type_name[40];
	const TskType *referenced_type;
};

const TskType *tsk_reference_referenced_type(const TskType *reference_type) {
	assert(tsk_reference_type_is_valid(reference_type));

	return ((const TskReferenceType *)reference_type)->referenced_type;
}
TskAny *tsk_reference_dereference(const TskType *reference_type, const TskReference *reference) {
	assert(tsk_reference_type_is_valid(reference_type));
	assert(reference != TSK_NULL);

	return *(TskAny *const *)reference;
}

// clang-format off
TSK_TYPE(tsk_reference_type_, TskReference,
	TSK_TYPE_TRAIT(tsk_reference_type_, TSK_TRAIT_ID_COMPLETE, &(TskTraitComplete){
		.size      = sizeof(TskAny *),
		.alignment = alignof(TskAny *),
	}),
	TSK_TYPE_TRAIT(tsk_reference_type_, TSK_TRAIT_ID_DROPPABLE, &(TskTraitDroppable){
		.drop = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_reference_type_, TSK_TRAIT_ID_CLONABLE, &(TskTraitClonable){
		.clone = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_reference_type_, TSK_TRAIT_ID_EQUATABLE, &(TskTraitEquatable){
		.equals = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_reference_type_, TSK_TRAIT_ID_HASHABLE, &(TskTraitHashable){
		.hash = TSK_NULL,
	})
);
// clang-format on

#define TSK_REFERENCE_TYPES_CAPACITY ((TskUSize)1 << 7)

TskReferenceType tsk_reference_types[TSK_REFERENCE_TYPES_CAPACITY];

TskBoolean tsk_reference_type_is_valid(const TskType *reference_type) {
	return tsk_type_is_valid(reference_type) &&
	       &tsk_reference_types[0] <= (const TskReferenceType *)reference_type && (const TskReferenceType *)reference_type < &tsk_reference_types[TSK_REFERENCE_TYPES_CAPACITY];
}
const TskType *tsk_reference_type(const TskType *referenced_type) {
	assert(tsk_type_is_valid(referenced_type));

	const TskType             *hasher_type = tsk_trait_builder_built_type(tsk_default_hasher_builder_type);
	alignas(max_align_t) TskU8 hasher[tsk_trait_complete_size(hasher_type)];
	tsk_trait_builder_build(tsk_default_hasher_builder_type, tsk_default_hasher_builder, hasher);

	tsk_trait_hasher_combine(hasher_type, hasher, (const TskU8 *)&referenced_type, sizeof(referenced_type)); // NOLINT(bugprone-sizeof-expression)
	TskU64 hash = tsk_trait_hasher_finalize(hasher_type, hasher);

	tsk_trait_droppable_drop(hasher_type, hasher);

	TskUSize starting_index = hash & (TSK_REFERENCE_TYPES_CAPACITY - 1);
	TskUSize index          = starting_index;
	while (tsk_reference_types[index].referenced_type != TSK_NULL) {
		if (tsk_reference_types[index].referenced_type == referenced_type) {
			return &tsk_reference_types[index].reference_type;
		}
		index = (index + 1) & (TSK_REFERENCE_TYPES_CAPACITY - 1);
		if (index == starting_index) {
			return TSK_NULL;
		}
	}

	tsk_reference_types[index].reference_type  = *tsk_reference_type_;
	tsk_reference_types[index].referenced_type = referenced_type;

	(void)snprintf(
	    tsk_reference_types[index].reference_type_name,
	    sizeof(tsk_reference_types[index].reference_type_name),
	    "TskReference<%s>",
	    tsk_type_name(referenced_type)
	);
	tsk_reference_types[index].reference_type.name = tsk_reference_types[index].reference_type_name;

	const TskType *reference_type                  = &tsk_reference_types[index].reference_type;

	assert(tsk_reference_type_is_valid(reference_type));

	return reference_type;
}

typedef struct TskReferenceConstType TskReferenceConstType;
struct TskReferenceConstType {
	TskType        reference_type;
	TskCharacter   reference_type_name[40];
	const TskType *referenced_type;
};

const TskType *tsk_reference_const_referenced_type(const TskType *reference_type) {
	assert(tsk_reference_const_type_is_valid(reference_type));

	return ((const TskReferenceConstType *)reference_type)->referenced_type;
}
const TskAny *tsk_reference_const_dereference(const TskType *reference_type, const TskReferenceConst *reference) {
	assert(tsk_reference_const_type_is_valid(reference_type));
	assert(reference != TSK_NULL);

	return *(const TskAny *const *)reference;
}

// clang-format off
TSK_TYPE(tsk_reference_const_type_, TskReferenceConst,
	TSK_TYPE_TRAIT(tsk_reference_const_type_, TSK_TRAIT_ID_COMPLETE, &(TskTraitComplete){
		.size      = sizeof(const TskAny *),
		.alignment = alignof(const TskAny *),
	}),
	TSK_TYPE_TRAIT(tsk_reference_const_type_, TSK_TRAIT_ID_DROPPABLE, &(TskTraitDroppable){
		.drop = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_reference_const_type_, TSK_TRAIT_ID_CLONABLE, &(TskTraitClonable){
		.clone = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_reference_const_type_, TSK_TRAIT_ID_EQUATABLE, &(TskTraitEquatable){
		.equals = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_reference_const_type_, TSK_TRAIT_ID_HASHABLE, &(TskTraitHashable){
		.hash = TSK_NULL,
	})
);
// clang-format on

#define TSK_REFERENCE_CONST_TYPES_CAPACITY ((TskUSize)1 << 7)

TskReferenceConstType tsk_reference_const_types[TSK_REFERENCE_CONST_TYPES_CAPACITY];

TskBoolean tsk_reference_const_type_is_valid(const TskType *reference_type) {
	return tsk_type_is_valid(reference_type) &&
	       &tsk_reference_const_types[0] <= (const TskReferenceConstType *)reference_type && (const TskReferenceConstType *)reference_type < &tsk_reference_const_types[TSK_REFERENCE_CONST_TYPES_CAPACITY];
}
const TskType *tsk_reference_const_type(const TskType *referenced_type) {
	assert(tsk_type_is_valid(referenced_type));

	const TskType             *hasher_type = tsk_trait_builder_built_type(tsk_default_hasher_builder_type);
	alignas(max_align_t) TskU8 hasher[tsk_trait_complete_size(hasher_type)];
	tsk_trait_builder_build(tsk_default_hasher_builder_type, tsk_default_hasher_builder, hasher);

	tsk_trait_hasher_combine(hasher_type, hasher, (const TskU8 *)&referenced_type, sizeof(referenced_type)); // NOLINT(bugprone-sizeof-expression)
	TskU64 hash = tsk_trait_hasher_finalize(hasher_type, hasher);

	tsk_trait_droppable_drop(hasher_type, hasher);

	TskUSize starting_index = hash & (TSK_REFERENCE_CONST_TYPES_CAPACITY - 1);
	TskUSize index          = starting_index;
	while (tsk_reference_const_types[index].referenced_type != TSK_NULL) {
		if (tsk_reference_const_types[index].referenced_type == referenced_type) {
			return &tsk_reference_const_types[index].reference_type;
		}
		index = (index + 1) & (TSK_REFERENCE_CONST_TYPES_CAPACITY - 1);
		if (index == starting_index) {
			return TSK_NULL;
		}
	}

	tsk_reference_const_types[index].reference_type  = *tsk_reference_const_type_;
	tsk_reference_const_types[index].referenced_type = referenced_type;

	(void)snprintf(tsk_reference_const_types[index].reference_type_name, sizeof(tsk_reference_const_types[index].reference_type_name), "TskReferenceConst<%s>", tsk_type_name(referenced_type));
	tsk_reference_const_types[index].reference_type.name = tsk_reference_const_types[index].reference_type_name;

	const TskType *reference_type                        = &tsk_reference_const_types[index].reference_type;

	assert(tsk_reference_const_type_is_valid(reference_type));

	return reference_type;
}
