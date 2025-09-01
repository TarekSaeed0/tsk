#include <tsk/default_hasher.h>

#include <tsk/trait/builder.h>
#include <tsk/trait/clonable.h>
#include <tsk/trait/complete.h>
#include <tsk/trait/droppable.h>
#include <tsk/trait/hasher.h>

typedef struct TskDefaultHasher TskDefaultHasher;
struct TskDefaultHasher {
	TskU64 hash;
};

TskEmpty tsk_default_hasher_combine(TskDefaultHasher *hasher, const TskU8 *bytes, TskUSize length) {
	for (TskUSize i = 0; i < length; i++) {
		hasher->hash ^= bytes[i];
		hasher->hash *= 1099511628211ULL;
	}
}
TskU64 tsk_default_hasher_finalize(const TskDefaultHasher *hasher) {
	return hasher->hash;
}

TskEmpty tsk_default_hasher_type_trait_hasher_combine(const TskType *hasher_type, TskAny *hasher, const TskU8 *bytes, TskUSize length) {
	(TskEmpty) hasher_type;
	tsk_default_hasher_combine(hasher, bytes, length);
}
TskU64 tsk_default_hasher_type_trait_hasher_finalize(const TskType *hasher_type, const TskAny *hasher) {
	(TskEmpty) hasher_type;
	return tsk_default_hasher_finalize(hasher);
}

// clang-format off
TSK_TYPE(tsk_default_hasher_type, TskDefaultHasher,
	TSK_TYPE_TRAIT(tsk_default_hasher_type, TSK_TRAIT_ID_COMPLETE, &(TskTraitComplete){
		.size = sizeof(TskDefaultHasher),
		.alignment = alignof(TskDefaultHasher),
	}),
	TSK_TYPE_TRAIT(tsk_default_hasher_type, TSK_TRAIT_ID_DROPPABLE, &(TskTraitDroppable){
		.drop = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_default_hasher_type, TSK_TRAIT_ID_CLONABLE, &(TskTraitClonable){
		.clone = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_default_hasher_type, TSK_TRAIT_ID_HASHER, &(TskTraitHasher){
		.combine = tsk_default_hasher_type_trait_hasher_combine,
		.finalize = tsk_default_hasher_type_trait_hasher_finalize,
	}),
);
// clang-format on

typedef struct TskDefaultHasherBuilder TskDefaultHasherBuilder;
struct TskDefaultHasherBuilder {
	TskUnit _;
};

const TskType *tsk_default_hasher_builder_built_type(TskEmpty) {
	return tsk_default_hasher_type;
}
TskEmpty tsk_default_hasher_builder_build(const TskDefaultHasherBuilder *hasher_builder, TskDefaultHasher *hasher) {
	(TskEmpty) hasher_builder;
	hasher->hash = 14695981039346656037ULL;
}

const TskDefaultHasherBuilder tsk_default_hasher_builder_;
const TskAny                 *tsk_default_hasher_builder = &tsk_default_hasher_builder_;

const TskType *tsk_default_hasher_builder_type_trait_builder_built_type(const TskType *builder_type) {
	(TskEmpty) builder_type;
	return tsk_default_hasher_builder_built_type();
}
TskEmpty tsk_default_hasher_builder_type_trait_builder_build(const TskType *builder_type, const TskAny *hasher_builder, TskAny *hasher) {
	(TskEmpty) builder_type;
	tsk_default_hasher_builder_build((const TskDefaultHasherBuilder *)hasher_builder, (TskDefaultHasher *)hasher);
}

// clang-format off
TSK_TYPE(tsk_default_hasher_builder_type_, TskDefaultHasherBuilder,
	TSK_TYPE_TRAIT(tsk_default_hasher_builder_type_, TSK_TRAIT_ID_COMPLETE, &(TskTraitComplete){
		.size = sizeof(TskDefaultHasherBuilder),
		.alignment = alignof(TskDefaultHasherBuilder),
	}),
	TSK_TYPE_TRAIT(tsk_default_hasher_builder_type_, TSK_TRAIT_ID_DROPPABLE, &(TskTraitDroppable){
		.drop = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_default_hasher_builder_type_, TSK_TRAIT_ID_CLONABLE, &(TskTraitClonable){
		.clone = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_default_hasher_builder_type_, TSK_TRAIT_ID_BUILDER, &(TskTraitBuilder){
		.built_type = tsk_default_hasher_builder_type_trait_builder_built_type,
		.build      = tsk_default_hasher_builder_type_trait_builder_build,
	}),
);
// clang-format on
const TskType *tsk_default_hasher_builder_type = tsk_default_hasher_builder_type_;
