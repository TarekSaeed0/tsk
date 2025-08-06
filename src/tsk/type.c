#include <tsk/type.h>

#include <assert.h>
#include <string.h>

TskBool tsk_type_is_valid(const TskType *type) {
	return type != TSK_NULL && type->name != TSK_NULL;
}
const TskChar *tsk_type_name(const TskType *type) {
	assert(tsk_type_is_valid(type));

	return type->name;
}
const TskAny *tsk_type_trait(const TskType *type, TskTrait trait) {
	assert(tsk_type_is_valid(type));
	assert(trait < TSK_TRAIT_COUNT);

	return type->traits[trait];
}
TskBool tsk_type_has_trait(const TskType *type, TskTrait trait) {
	assert(tsk_type_is_valid(type));
	assert(trait < TSK_TRAIT_COUNT);

	return tsk_type_trait(type, trait) != TSK_NULL;
}

static inline TskBool tsk_trait_complete_is_valid(const TskTraitComplete *trait_complete) {
	return trait_complete != TSK_NULL && trait_complete->alignment > 0 &&
	       (trait_complete->alignment & (trait_complete->alignment - 1)) == 0;
}
TskUSize tsk_trait_complete_size(const TskTraitComplete *trait_complete) {
	assert(tsk_trait_complete_is_valid(trait_complete));

	return trait_complete->size;
}
TskUSize tsk_trait_complete_alignment(const TskTraitComplete *trait_complete) {
	assert(tsk_trait_complete_is_valid(trait_complete));

	return trait_complete->alignment;
}

static inline TskBool tsk_trait_droppable_is_valid(const TskTraitDroppable *trait_droppable) {
	return trait_droppable != TSK_NULL;
}
TskEmpty tsk_trait_droppable_drop(const TskTraitDroppable *trait_droppable, TskAny *droppable) {
	assert(tsk_trait_droppable_is_valid(trait_droppable));
	assert(droppable != TSK_NULL);

	if (trait_droppable->drop != TSK_NULL) {
		trait_droppable->drop(droppable);
	}
}

static inline TskBool tsk_trait_clonable_is_valid(const TskTraitClonable *trait_clonable) {
	return trait_clonable != TSK_NULL && tsk_trait_complete_is_valid(trait_clonable->trait_complete);
}
TskBool tsk_trait_clonable_clone(const TskTraitClonable *trait_clonable, const TskAny *clonable_1, TskAny *clonable_2) {
	assert(tsk_trait_clonable_is_valid(trait_clonable));
	assert(clonable_1 != TSK_NULL);
	assert(clonable_2 != TSK_NULL);

	if (trait_clonable->clone != TSK_NULL) {
		return trait_clonable->clone(clonable_1, clonable_2);
	}

	TskUSize size = tsk_trait_complete_size(trait_clonable->trait_complete);
	if (size != 0) {
		memcpy(
		    clonable_2,
		    clonable_1,
		    size
		);
	}

	return TSK_TRUE;
}

static inline TskBool tsk_trait_comparable_is_valid(const TskTraitComparable *trait_comparable) {
	return trait_comparable != TSK_NULL && trait_comparable->compare != TSK_NULL;
}
TskOrdering tsk_trait_comparable_compare(const TskTraitComparable *trait_comparable, const TskAny *comparable_1, const TskAny *comparable_2) {
	assert(tsk_trait_comparable_is_valid(trait_comparable));
	assert(comparable_1 != TSK_NULL);
	assert(comparable_2 != TSK_NULL);

	return trait_comparable->compare(comparable_1, comparable_2);
}

static inline TskBool tsk_trait_equatable_is_valid(const TskTraitEquatable *trait_equatable) {
	return trait_equatable != TSK_NULL && trait_equatable->equals != TSK_NULL;
}
TskBool tsk_trait_equatable_equals(const TskTraitEquatable *trait_equatable, const TskAny *equatable_1, const TskAny *equatable_2) {
	assert(tsk_trait_equatable_is_valid(trait_equatable));
	assert(equatable_1 != TSK_NULL);
	assert(equatable_2 != TSK_NULL);

	return trait_equatable->equals(equatable_1, equatable_2);
}

static inline TskBool tsk_trait_hasher_is_valid(const TskTraitHasher *trait_hasher) {
	return trait_hasher != TSK_NULL && trait_hasher->combine != TSK_NULL && trait_hasher->finalize != TSK_NULL;
}
TskEmpty tsk_trait_hasher_combine(const TskTraitHasher *trait_hasher, TskAny *hasher, const TskU8 *bytes, TskUSize length) {
	assert(tsk_trait_hasher_is_valid(trait_hasher));

	trait_hasher->combine(hasher, bytes, length);
}
TskU64 tsk_trait_hasher_finalize(const TskTraitHasher *trait_hasher, const TskAny *hasher) {
	assert(tsk_trait_hasher_is_valid(trait_hasher));
	assert(hasher != TSK_NULL);

	return trait_hasher->finalize(hasher);
}

static inline TskBool tsk_trait_hashable_is_valid(const TskTraitHashable *trait_hashable) {
	return trait_hashable != TSK_NULL && trait_hashable->hash != TSK_NULL;
}
TskEmpty tsk_trait_hashable_hash(const TskTraitHashable *trait_hashable, const TskAny *hashable, const TskTraitHasher *trait_hasher, TskAny *hasher) {
	assert(tsk_trait_hashable_is_valid(trait_hashable));
	assert(hashable != TSK_NULL);
	assert(tsk_trait_hasher_is_valid(trait_hasher));
	assert(hasher != TSK_NULL);

	trait_hashable->hash(hashable, trait_hasher, hasher);
}

static inline TskBool tsk_trait_builder_is_valid(const TskTraitBuilder *trait_builder) {
	return trait_builder != TSK_NULL && tsk_type_is_valid(trait_builder->built_type) &&
	       tsk_type_has_trait(trait_builder->built_type, TSK_TRAIT_COMPLETE) && trait_builder->build != TSK_NULL;
}
const TskType *tsk_trait_builder_built_type(const TskTraitBuilder *trait_builder) {
	assert(tsk_trait_builder_is_valid(trait_builder));

	return trait_builder->built_type;
}
TskEmpty tsk_trait_builder_build(const TskTraitBuilder *trait_builder, const TskAny *builder, TskAny *built) {
	assert(tsk_trait_builder_is_valid(trait_builder));
	assert(builder != TSK_NULL);
	assert(built != TSK_NULL);

	trait_builder->build(builder, built);
}

TskEmpty tsk_default_hasher_combine(TskDefaultHasher *hasher, const TskU8 *bytes, TskUSize length) {
	for (TskUSize i = 0; i < length; i++) {
		hasher->hash ^= bytes[i];
		hasher->hash *= 1099511628211ULL;
	}
}
TskU64 tsk_default_hasher_finalize(const TskDefaultHasher *hasher) {
	return hasher->hash;
}

static TskEmpty tsk_default_hasher_type_combine(TskAny *hasher, const TskU8 *bytes, TskUSize length) {
	tsk_default_hasher_combine(hasher, bytes, length);
}
static TskU64 tsk_default_hasher_type_finalize(const TskAny *hasher) {
	return tsk_default_hasher_finalize(hasher);
}
const TskType tsk_default_hasher_type = {
	.name   = "TskDefaultHasher",
	.traits = {
	    [TSK_TRAIT_COMPLETE] = &(TskTraitComplete){
	        .size      = sizeof(TskDefaultHasher),
	        .alignment = alignof(TskDefaultHasher),
	    },
	    [TSK_TRAIT_DROPPABLE] = &(TskTraitDroppable){
	        .drop = TSK_NULL,
	    },
	    [TSK_TRAIT_HASHER] = &(TskTraitHasher){
	        .combine  = tsk_default_hasher_type_combine,
	        .finalize = tsk_default_hasher_type_finalize,
	    },
	},
};

TskEmpty tsk_default_hasher_builder_build(const TskDefaultHasherBuilder *hasher_builder, TskDefaultHasher *hasher) {
	(void)hasher_builder;

	hasher->hash = 14695981039346656037ULL;
}

TskDefaultHasher tsk_default_hasher_builder; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

static const TskTraitComplete tsk_default_hasher_builder_trait_complete = {
	.size      = sizeof(TskDefaultHasherBuilder),
	.alignment = alignof(TskDefaultHasherBuilder),
};

static const TskTraitDroppable tsk_default_hasher_builder_trait_droppable = {
	.drop = TSK_NULL,
};

static TskEmpty tsk_default_hasher_builder_trait_builder_build(const TskAny *hasher_builder, TskAny *hasher) {
	tsk_default_hasher_builder_build(hasher_builder, (TskDefaultHasher *)hasher);
}
static const TskTraitBuilder tsk_default_hasher_builder_trait_builder = {
	.built_type = &tsk_default_hasher_type,
	.build      = tsk_default_hasher_builder_trait_builder_build,
};

const TskType tsk_default_hasher_builder_type = {
	.name   = "TskDefaultHasherBuilder",
	.traits = {
	    [TSK_TRAIT_COMPLETE]  = &tsk_default_hasher_builder_trait_complete,
	    [TSK_TRAIT_DROPPABLE] = &tsk_default_hasher_builder_trait_droppable,
	    [TSK_TRAIT_BUILDER]   = &tsk_default_hasher_builder_trait_builder,
	},
};

TskUnit tsk_unit; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

TskOrdering tsk_unit_compare(TskUnit unit_1, TskUnit unit_2) {
	(void)unit_1, (void)unit_2;
	return TSK_ORDERING_EQUAL;
}
TskBool tsk_unit_equals(TskUnit unit_1, TskUnit unit_2) {
	(void)unit_1, (void)unit_2;
	return TSK_TRUE;
}
TskEmpty tsk_unit_hash(TskUnit unit, const TskTraitHasher *trait_hasher, TskAny *hasher) {
	(void)unit, (void)trait_hasher, (void)hasher;
}

static const TskTraitComplete tsk_unit_trait_complete = {
	.size      = sizeof(TskUnit),
	.alignment = alignof(TskUnit),
};

static const TskTraitDroppable tsk_unit_trait_droppable = {
	.drop = TSK_NULL,
};

static const TskTraitClonable tsk_unit_trait_clonable = {
	.trait_complete = &tsk_unit_trait_complete,
	.clone          = TSK_NULL,
};

static TskOrdering tsk_unit_trait_comparable_compare(const TskAny *unit_1, const TskAny *unit_2) {
	return tsk_unit_compare(*(const TskUnit *)unit_1, *(const TskUnit *)unit_2);
}
static const TskTraitComparable tsk_unit_trait_comparable = {
	.compare = tsk_unit_trait_comparable_compare,
};

static TskBool tsk_unit_type_equatable_equals(const TskAny *unit_1, const TskAny *unit_2) {
	return tsk_unit_equals(*(const TskUnit *)unit_1, *(const TskUnit *)unit_2);
}
static const TskTraitEquatable tsk_unit_trait_equatable = {
	.equals = tsk_unit_type_equatable_equals,
};

static TskEmpty tsk_unit_trait_hashable_hash(const TskAny *unit, const TskTraitHasher *trait_hasher, TskAny *hasher) {
	tsk_unit_hash(*(const TskUnit *)unit, trait_hasher, hasher);
}
static const TskTraitHashable tsk_unit_trait_hashable = {
	.hash = tsk_unit_trait_hashable_hash,
};

const TskType tsk_unit_type = {
	.name   = "TskUnit",
	.traits = {
	    [TSK_TRAIT_COMPLETE]   = &tsk_unit_trait_complete,
	    [TSK_TRAIT_DROPPABLE]  = &tsk_unit_trait_droppable,
	    [TSK_TRAIT_CLONABLE]   = &tsk_unit_trait_clonable,
	    [TSK_TRAIT_COMPARABLE] = &tsk_unit_trait_comparable,
	    [TSK_TRAIT_EQUATABLE]  = &tsk_unit_trait_equatable,
	    [TSK_TRAIT_HASHABLE]   = &tsk_unit_trait_hashable,
	},
};

#define TSK_INTEGRAL_DEFINITION(integral_prefix, integral_parameter, IntegralType)                                                              \
	TskOrdering integral_prefix##_compare(IntegralType integral_parameter##_1, IntegralType integral_parameter##_2) {                             \
		return (integral_parameter##_1 > integral_parameter##_2) - (integral_parameter##_1 < integral_parameter##_2);                               \
	}                                                                                                                                             \
	TskBool integral_prefix##_equals(IntegralType integral_parameter##_1, IntegralType integral_parameter##_2) {                                  \
		return integral_parameter##_1 == integral_parameter##_2;                                                                                    \
	}                                                                                                                                             \
	TskEmpty integral_prefix##_hash(IntegralType integral_parameter, const TskTraitHasher *trait_hasher, TskAny *hasher) {                        \
		tsk_trait_hasher_combine(                                                                                                                   \
		    trait_hasher,                                                                                                                           \
		    hasher,                                                                                                                                 \
		    (const TskU8 *)&integral_parameter,                                                                                                     \
		    sizeof(integral_parameter)                                                                                                              \
		);                                                                                                                                          \
	}                                                                                                                                             \
                                                                                                                                                \
	static const TskTraitComplete integral_prefix##_trait_complete = {                                                                            \
		.size      = sizeof(IntegralType),                                                                                                          \
		.alignment = alignof(IntegralType),                                                                                                         \
	};                                                                                                                                            \
                                                                                                                                                \
	static const TskTraitDroppable integral_prefix##_trait_droppable = {                                                                          \
		.drop = TSK_NULL,                                                                                                                           \
	};                                                                                                                                            \
                                                                                                                                                \
	static const TskTraitClonable integral_prefix##_trait_clonable = {                                                                            \
		.trait_complete = &integral_prefix##_trait_complete,                                                                                        \
		.clone          = TSK_NULL,                                                                                                                 \
	};                                                                                                                                            \
                                                                                                                                                \
	static TskOrdering integral_prefix##_trait_comparable_compare(const TskAny *integral_parameter##_1, const TskAny *integral_parameter##_2) {   \
		return integral_prefix##_compare(*(const IntegralType *)integral_parameter##_1, *(const IntegralType *)integral_parameter##_2);             \
	}                                                                                                                                             \
	static const TskTraitComparable integral_prefix##_trait_comparable = {                                                                        \
		.compare = integral_prefix##_trait_comparable_compare,                                                                                      \
	};                                                                                                                                            \
                                                                                                                                                \
	static TskBool integral_prefix##_type_equatable_equals(const TskAny *integral_parameter##_1, const TskAny *integral_parameter##_2) {          \
		return integral_prefix##_equals(*(const IntegralType *)integral_parameter##_1, *(const IntegralType *)integral_parameter##_2);              \
	}                                                                                                                                             \
	static const TskTraitEquatable integral_prefix##_trait_equatable = {                                                                          \
		.equals = integral_prefix##_type_equatable_equals,                                                                                          \
	};                                                                                                                                            \
                                                                                                                                                \
	static TskEmpty integral_prefix##_trait_hashable_hash(const TskAny *integral_parameter, const TskTraitHasher *trait_hasher, TskAny *hasher) { \
		integral_prefix##_hash(*(const IntegralType *)integral_parameter, trait_hasher, hasher);                                                    \
	}                                                                                                                                             \
	static const TskTraitHashable integral_prefix##_trait_hashable = {                                                                            \
		.hash = integral_prefix##_trait_hashable_hash,                                                                                              \
	};                                                                                                                                            \
                                                                                                                                                \
	const TskType integral_prefix##_type = {                                                                                                      \
		.name   = #IntegralType,                                                                                                                    \
		.traits = {                                                                                                                                 \
		    [TSK_TRAIT_COMPLETE]   = &integral_prefix##_trait_complete,                                                                             \
		    [TSK_TRAIT_DROPPABLE]  = &integral_prefix##_trait_droppable,                                                                            \
		    [TSK_TRAIT_CLONABLE]   = &integral_prefix##_trait_clonable,                                                                             \
		    [TSK_TRAIT_COMPARABLE] = &integral_prefix##_trait_comparable,                                                                           \
		    [TSK_TRAIT_EQUATABLE]  = &integral_prefix##_trait_equatable,                                                                            \
		    [TSK_TRAIT_HASHABLE]   = &integral_prefix##_trait_hashable,                                                                             \
		},                                                                                                                                          \
	}

TSK_INTEGRAL_DEFINITION(tsk_bool, boolean, TskBool);

TSK_INTEGRAL_DEFINITION(tsk_char, character, TskChar);

TSK_INTEGRAL_DEFINITION(tsk_u8, u8, TskU8);
TSK_INTEGRAL_DEFINITION(tsk_u16, u16, TskU16);
TSK_INTEGRAL_DEFINITION(tsk_u32, u32, TskU32);
TSK_INTEGRAL_DEFINITION(tsk_u64, u64, TskU64);
TSK_INTEGRAL_DEFINITION(tsk_usize, usize, TskUSize);

TSK_INTEGRAL_DEFINITION(tsk_i8, i8, TskI8);
TSK_INTEGRAL_DEFINITION(tsk_i16, i16, TskI16);
TSK_INTEGRAL_DEFINITION(tsk_i32, i32, TskI32);
TSK_INTEGRAL_DEFINITION(tsk_i64, i64, TskI64);
TSK_INTEGRAL_DEFINITION(tsk_isize, isize, TskISize);

#undef TSK_INTEGRAL_DEFINITION

#define TSK_FLOATING_POINT_DEFINITION(floating_point_prefix, floating_point_parameter, FloatingPointType)                                                       \
	TskOrdering floating_point_prefix##_compare(FloatingPointType floating_point_parameter##_1, FloatingPointType floating_point_parameter##_2) {                 \
		return (floating_point_parameter##_1 > floating_point_parameter##_2) - (floating_point_parameter##_1 < floating_point_parameter##_2);                       \
	}                                                                                                                                                             \
	TskBool floating_point_prefix##_equals(FloatingPointType floating_point_parameter##_1, FloatingPointType floating_point_parameter##_2) {                      \
		return floating_point_parameter##_1 == floating_point_parameter##_2;                                                                                        \
	}                                                                                                                                                             \
	TskEmpty floating_point_prefix##_hash(FloatingPointType floating_point_parameter, const TskTraitHasher *trait_hasher, TskAny *hasher) {                       \
		tsk_trait_hasher_combine(                                                                                                                                   \
		    trait_hasher,                                                                                                                                           \
		    hasher,                                                                                                                                                 \
		    (const TskU8 *)&floating_point_parameter,                                                                                                               \
		    sizeof(floating_point_parameter)                                                                                                                        \
		);                                                                                                                                                          \
	}                                                                                                                                                             \
                                                                                                                                                                \
	static const TskTraitComplete floating_point_prefix##_trait_complete = {                                                                                      \
		.size      = sizeof(FloatingPointType),                                                                                                                     \
		.alignment = alignof(FloatingPointType),                                                                                                                    \
	};                                                                                                                                                            \
                                                                                                                                                                \
	static const TskTraitDroppable floating_point_prefix##_trait_droppable = {                                                                                    \
		.drop = TSK_NULL,                                                                                                                                           \
	};                                                                                                                                                            \
                                                                                                                                                                \
	static const TskTraitClonable floating_point_prefix##_trait_clonable = {                                                                                      \
		.trait_complete = &floating_point_prefix##_trait_complete,                                                                                                  \
		.clone          = TSK_NULL,                                                                                                                                 \
	};                                                                                                                                                            \
                                                                                                                                                                \
	static TskOrdering floating_point_prefix##_trait_comparable_compare(const TskAny *floating_point_parameter##_1, const TskAny *floating_point_parameter##_2) { \
		return floating_point_prefix##_compare(*(const FloatingPointType *)floating_point_parameter##_1, *(const FloatingPointType *)floating_point_parameter##_2); \
	}                                                                                                                                                             \
	static const TskTraitComparable floating_point_prefix##_trait_comparable = {                                                                                  \
		.compare = floating_point_prefix##_trait_comparable_compare,                                                                                                \
	};                                                                                                                                                            \
                                                                                                                                                                \
	static TskBool floating_point_prefix##_type_equatable_equals(const TskAny *floating_point_parameter##_1, const TskAny *floating_point_parameter##_2) {        \
		return floating_point_prefix##_equals(*(const FloatingPointType *)floating_point_parameter##_1, *(const FloatingPointType *)floating_point_parameter##_2);  \
	}                                                                                                                                                             \
	static const TskTraitEquatable floating_point_prefix##_trait_equatable = {                                                                                    \
		.equals = floating_point_prefix##_type_equatable_equals,                                                                                                    \
	};                                                                                                                                                            \
                                                                                                                                                                \
	static TskEmpty floating_point_prefix##_trait_hashable_hash(const TskAny *floating_point_parameter, const TskTraitHasher *trait_hasher, TskAny *hasher) {     \
		floating_point_prefix##_hash(*(const FloatingPointType *)floating_point_parameter, trait_hasher, hasher);                                                   \
	}                                                                                                                                                             \
	static const TskTraitHashable floating_point_prefix##_trait_hashable = {                                                                                      \
		.hash = floating_point_prefix##_trait_hashable_hash,                                                                                                        \
	};                                                                                                                                                            \
                                                                                                                                                                \
	const TskType floating_point_prefix##_type = {                                                                                                                \
		.name   = #FloatingPointType,                                                                                                                               \
		.traits = {                                                                                                                                                 \
		    [TSK_TRAIT_COMPLETE]   = &floating_point_prefix##_trait_complete,                                                                                       \
		    [TSK_TRAIT_DROPPABLE]  = &floating_point_prefix##_trait_droppable,                                                                                      \
		    [TSK_TRAIT_CLONABLE]   = &floating_point_prefix##_trait_clonable,                                                                                       \
		    [TSK_TRAIT_COMPARABLE] = &floating_point_prefix##_trait_comparable,                                                                                     \
		    [TSK_TRAIT_EQUATABLE]  = &floating_point_prefix##_trait_equatable,                                                                                      \
		    [TSK_TRAIT_HASHABLE]   = &floating_point_prefix##_trait_hashable,                                                                                       \
		},                                                                                                                                                          \
	}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
TSK_FLOATING_POINT_DEFINITION(tsk_f32, f32, TskF32);
TSK_FLOATING_POINT_DEFINITION(tsk_f64, f64, TskF64);
#pragma GCC diagnostic pop

#undef TSK_FLOATING_POINT_DECLARATION
