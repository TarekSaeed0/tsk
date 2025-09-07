#ifndef TSK_TYPE_H_INCLUDED
#define TSK_TYPE_H_INCLUDED

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
	#pragma GCC diagnostic ignored "-Wpedantic"
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

typedef bool TskBoolean;

typedef char TskCharacter;

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

typedef TskUSize TskTraitID;

typedef struct TskTypeTraitTableEntry TskTypeTraitTableEntry;
struct TskTypeTraitTableEntry {
	TskTraitID    trait_id;
	const TskAny *trait_data;
};
typedef struct TskTypeTraitTable TskTypeTraitTable;
struct TskTypeTraitTable {
	TskTypeTraitTableEntry *entries;
	TskUSize                capacity;
};
typedef struct TskType TskType;
struct TskType {
	const TskCharacter      *name;
	const TskTypeTraitTable *trait_table;
};
TskBoolean          tsk_type_is_valid(const TskType *type);
const TskCharacter *tsk_type_name(const TskType *type);
const TskAny       *tsk_type_trait(const TskType *type, TskTraitID trait_id);
TskBoolean          tsk_type_has_trait(const TskType *type, TskTraitID trait_id);

// clang-format off
#define TSK_TYPE_TRAIT_TABLE_LENGTH_(\
	_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, \
	_11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
	_21, _22, _23, _24, _25, _26, _27, _28, _29, _30, \
	_31, _32, _33, _34, _35, _36, _37, _38, _39, _40, \
	_41, _42, _43, _44, _45, _46, _47, _48, _49, _50, \
	_51, _52, _53, _54, _55, _56, _57, _58, _59, _60, \
	_61, _62, _63, n, ...) n
#define TSK_TYPE_TRAIT_TABLE_LENGTH(...) \
	TSK_TYPE_TRAIT_TABLE_LENGTH_(\
		__VA_ARGS__, 63, 62, 61, 60, \
		59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
		49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
		39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
		29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
		19, 18, 17, 16, 15, 14, 13, 12, 11, 10, \
		9, 8, 7, 6, 5, 4, 3, 2, 1, 0 \
	)
// clang-format on

#define TSK_TYPE_TRAIT_TABLE_CAPACITY_(length) \
	(length <= 1 ? 1 : (1ULL << (64 - __builtin_clzll((2 * length) - 1))))
#define TSK_TYPE_TRAIT_TABLE_CAPACITY(...) \
	TSK_TYPE_TRAIT_TABLE_CAPACITY_(TSK_TYPE_TRAIT_TABLE_LENGTH(__VA_ARGS__))

#define TSK_TYPE(type_identifier, type_name, ...)                                                    \
	enum {                                                                                             \
		type_identifier##_trait_table_capacity = TSK_TYPE_TRAIT_TABLE_CAPACITY(__VA_ARGS__)              \
	};                                                                                                 \
	const TskType type_identifier##_ = {                                                               \
		.name        = #type_name,                                                                       \
		.trait_table = &(const TskTypeTraitTable){                                                       \
		    .entries  = (TskTypeTraitTableEntry[type_identifier##_trait_table_capacity]){ __VA_ARGS__ }, \
		    .capacity = type_identifier##_trait_table_capacity,                                          \
		}                                                                                                \
	};                                                                                                 \
	const TskType *const type_identifier = &type_identifier##_
#define TSK_TYPE_TRAIT(type_identifier, trait_id, ...) \
	[(trait_id) & (type_identifier##_trait_table_capacity - 1)] = { (trait_id), (__VA_ARGS__) }

extern const TskType *const tsk_unit_type;

extern const TskType *const tsk_boolean_type;

extern const TskType *const tsk_character_type;

extern const TskType *const tsk_u8_type;
extern const TskType *const tsk_u16_type;
extern const TskType *const tsk_u32_type;
extern const TskType *const tsk_u64_type;
extern const TskType *const tsk_usize_type;

extern const TskType *const tsk_i8_type;
extern const TskType *const tsk_i16_type;
extern const TskType *const tsk_i32_type;
extern const TskType *const tsk_i64_type;
extern const TskType *const tsk_isize_type;

extern const TskType *const tsk_f32_type;
extern const TskType *const tsk_f64_type;

#ifdef __cplusplus
}
#endif

#endif // TSK_TYPE_H_INCLUDED
