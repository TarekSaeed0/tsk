#ifndef TSK_CORE_H_INCLUDED
#define TSK_CORE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef void TskEmpty;
#if defined(__GNUC__) || defined(__clang__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wgnu-empty-struct"
typedef struct {
} TskUnit;
	#pragma GCC diagnostic pop
#else
typedef struct {
	char _;
} TskUnit;
#endif

#define TSK_NULL NULL

#define TSK_FALSE false
#define TSK_TRUE true

typedef void TskAny;

typedef bool TskBool;

typedef char TskChar;

typedef uint8_t  TskU8;
typedef uint16_t TskU16;
typedef uint32_t TskU32;
typedef uint64_t TskU64;
typedef size_t   TskUSize;

typedef int8_t    TskI8;
typedef int16_t   TskI16;
typedef int32_t   TskI32;
typedef int64_t   TskI64;
typedef ptrdiff_t TskISize;

typedef float  TskF32;
typedef double TskF64;

typedef enum TskTrait {
	TSK_TRAIT_COMPLETE,
	TSK_TRAIT_DROPPABLE,
	TSK_TRAIT_CLONABLE,
	TSK_TRAIT_COMPARABLE,
	TSK_TRAIT_EQUATABLE,
	TSK_TRAIT_HASHER,
	TSK_TRAIT_HASHABLE,
	TSK_TRAIT_BUILDER,

	TSK_TRAIT_COUNT,
} TskTrait;

typedef struct TskType TskType;
struct TskType {
	const TskChar *name;
	const TskAny  *traits[TSK_TRAIT_COUNT];
};
TskBool        tsk_type_is_valid(const TskType *type);
const TskChar *tsk_type_name(const TskType *type);
const TskAny  *tsk_type_trait(const TskType *type, TskTrait trait);
TskBool        tsk_type_has_trait(const TskType *type, TskTrait trait);

typedef struct TskTraitComplete TskTraitComplete;
struct TskTraitComplete {
	TskUSize size;
	TskUSize alignment;
};
TskUSize tsk_trait_complete_size(const TskTraitComplete *trait_complete);
TskUSize tsk_trait_complete_alignment(const TskTraitComplete *trait_complete);

typedef struct TskTraitDroppable TskTraitDroppable;
struct TskTraitDroppable {
	TskEmpty (*drop)(TskAny *droppable);
};
TskEmpty tsk_trait_droppable_drop(const TskTraitDroppable *trait_droppable, TskAny *droppable);

typedef struct TskTraitClonable TskTraitClonable;
struct TskTraitClonable {
	const TskTraitComplete *trait_complete;
	TskBool (*clone)(const TskAny *clonable_1, TskAny *clonable_2);
};
TskBool tsk_trait_clonable_clone(const TskTraitClonable *trait_clonable, const TskAny *clonable_1, TskAny *clonable_2);

typedef enum TskOrdering {
	TSK_ORDERING_LESS    = -1,
	TSK_ORDERING_EQUAL   = 0,
	TSK_ORDERING_GREATER = 1
} TskOrdering;

typedef struct TskTraitComparable TskTraitComparable;
struct TskTraitComparable {
	TskOrdering (*compare)(const TskAny *comparable_1, const TskAny *comparable_2);
};
TskOrdering tsk_trait_comparable_compare(const TskTraitComparable *trait_comparable, const TskAny *comparable_1, const TskAny *comparable_2);

typedef struct TskTraitEquatable TskTraitEquatable;
struct TskTraitEquatable {
	TskBool (*equals)(const TskAny *equatable_1, const TskAny *equatable_2);
};
TskBool tsk_trait_equatable_equals(const TskTraitEquatable *trait_equatable, const TskAny *equatable_1, const TskAny *equatable_2);

typedef struct TskTraitHasher TskTraitHasher;
struct TskTraitHasher {
	TskEmpty (*combine)(TskAny *hasher, const TskU8 *bytes, TskUSize length);
	TskU64 (*finalize)(const TskAny *hasher);
};
TskEmpty tsk_trait_hasher_combine(const TskTraitHasher *trait_hasher, TskAny *hasher, const TskU8 *bytes, TskUSize length);
TskU64   tsk_trait_hasher_finalize(const TskTraitHasher *trait_hasher, const TskAny *hasher);

typedef struct TskTraitHashable TskTraitHashable;
struct TskTraitHashable {
	TskEmpty (*hash)(const TskAny *hashable, const TskTraitHasher *trait_hasher, TskAny *hasher);
};
TskEmpty tsk_trait_hashable_hash(const TskTraitHashable *trait_hashable, const TskAny *hashable, const TskTraitHasher *trait_hasher, TskAny *hasher);

typedef struct TskTraitBuilder TskTraitBuilder;
struct TskTraitBuilder {
	const TskType *built_type;
	TskEmpty (*build)(const TskAny *builder, TskAny *hasher);
};
const TskType *tsk_trait_builder_built_type(const TskTraitBuilder *trait_builder);
TskEmpty       tsk_trait_builder_build(const TskTraitBuilder *trait_builder, const TskAny *builder, TskAny *built);

typedef struct TskDefaultHasher TskDefaultHasher;
struct TskDefaultHasher {
	TskU64 hash;
};
TskEmpty tsk_default_hasher_combine(TskDefaultHasher *hasher, const TskU8 *bytes, TskUSize length);
TskU64   tsk_default_hasher_finalize(const TskDefaultHasher *hasher);

extern const TskType tsk_default_hasher_type;

typedef struct TskDefaultHasherBuilder TskDefaultHasherBuilder;
struct TskDefaultHasherBuilder {
	TskUnit _;
};
TskEmpty tsk_default_hasher_builder_build(const TskDefaultHasherBuilder *hasher_builder, TskDefaultHasher *hasher);

extern TskDefaultHasher tsk_default_hasher_builder; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

extern const TskType tsk_default_hasher_builder_type;

extern TskUnit tsk_unit; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

TskOrdering tsk_unit_compare(TskUnit unit_1, TskUnit unit_2);
TskBool     tsk_unit_equals(TskUnit unit_1, TskUnit unit_2);
TskEmpty    tsk_unit_hash(TskUnit unit, const TskTraitHasher *trait_hasher, TskAny *hasher);

extern const TskType tsk_unit_type;

#define TSK_INTEGRAL_DECLARATION(integral_prefix, integral_parameter, IntegralType)                                        \
	TskOrdering integral_prefix##_compare(IntegralType integral_parameter##_1, IntegralType integral_parameter##_2);         \
	TskBool     integral_prefix##_equals(IntegralType integral_parameter##_1, IntegralType integral_parameter##_2);          \
	TskEmpty    integral_prefix##_hash(IntegralType integral_parameter, const TskTraitHasher *trait_hasher, TskAny *hasher); \
                                                                                                                           \
	extern const TskType integral_prefix##_type

TSK_INTEGRAL_DECLARATION(tsk_bool, boolean, TskBool);

TSK_INTEGRAL_DECLARATION(tsk_char, character, TskChar);

TSK_INTEGRAL_DECLARATION(tsk_u8, u8, TskU8);
TSK_INTEGRAL_DECLARATION(tsk_u16, u16, TskU16);
TSK_INTEGRAL_DECLARATION(tsk_u32, u32, TskU32);
TSK_INTEGRAL_DECLARATION(tsk_u64, u64, TskU64);
TSK_INTEGRAL_DECLARATION(tsk_usize, usize, TskUSize);

TSK_INTEGRAL_DECLARATION(tsk_i8, i8, TskI8);
TSK_INTEGRAL_DECLARATION(tsk_i16, i16, TskI16);
TSK_INTEGRAL_DECLARATION(tsk_i32, i32, TskI32);
TSK_INTEGRAL_DECLARATION(tsk_i64, i64, TskI64);
TSK_INTEGRAL_DECLARATION(tsk_isize, isize, TskISize);

#undef TSK_INTEGRAL_DECLARATION

#define TSK_FLOATING_POINT_DECLARATION(floating_point_prefix, floating_point_parameter, FloatingPointType)                                     \
	TskOrdering floating_point_prefix##_compare(FloatingPointType floating_point_parameter##_1, FloatingPointType floating_point_parameter##_2); \
	TskBool     floating_point_prefix##_equals(FloatingPointType floating_point_parameter##_1, FloatingPointType floating_point_parameter##_2);  \
	TskEmpty    floating_point_prefix##_hash(FloatingPointType floating_point_parameter, const TskTraitHasher *trait_hasher, TskAny *hasher);    \
                                                                                                                                               \
	extern const TskType floating_point_prefix##_type

TSK_FLOATING_POINT_DECLARATION(tsk_f32, f32, TskF32);
TSK_FLOATING_POINT_DECLARATION(tsk_f64, f64, TskF64);

#undef TSK_FLOATING_POINT_DECLARATION

#ifdef __cplusplus
}
#endif

#endif // TSK_CORE_H_INCLUDED
