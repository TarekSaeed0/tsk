#include <tsk/type.h>

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

TskBoolean tsk_type_is_valid(const TskType *type) {
	return type != TSK_NULL && type->name != TSK_NULL &&
	       type->trait_table != TSK_NULL && (type->trait_table->entries != TSK_NULL || type->trait_table->capacity == 0) && (type->trait_table->capacity & (type->trait_table->capacity - 1)) == 0;
}
const TskCharacter *tsk_type_name(const TskType *type) {
	assert(tsk_type_is_valid(type));

	return type->name;
}
const TskAny *tsk_type_trait(const TskType *type, TskTraitID trait_id) {
	assert(tsk_type_is_valid(type));

	TskUSize starting_index = trait_id & (type->trait_table->capacity - 1);
	TskUSize index          = starting_index;
	while (type->trait_table->entries[index].trait_data != TSK_NULL) {
		if (type->trait_table->entries[index].trait_id == trait_id) {
			return type->trait_table->entries[index].trait_data;
		}
		index = (index + 1) & (type->trait_table->capacity - 1);
		if (index == starting_index) {
			break;
		}
	}

	return TSK_NULL;
}
TskBoolean tsk_type_has_trait(const TskType *type, TskTraitID trait_id) {
	return tsk_type_trait(type, trait_id) != TSK_NULL;
}

// clang-format off
TSK_TYPE(tsk_unit_type, TskUnit,
	TSK_TYPE_TRAIT(tsk_unit_type, TSK_TRAIT_ID_COMPLETE, &(TskTraitComplete){
		.size      = sizeof(TskUnit),
		.alignment = alignof(TskUnit),
	}),
	TSK_TYPE_TRAIT(tsk_unit_type, TSK_TRAIT_ID_DROPPABLE, &(TskTraitDroppable){
		.drop = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_unit_type, TSK_TRAIT_ID_CLONABLE, &(TskTraitClonable){
		.clone = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_unit_type, TSK_TRAIT_ID_EQUATABLE, &(TskTraitEquatable){
		.equals = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_unit_type, TSK_TRAIT_ID_HASHABLE, &(TskTraitHashable){
		.hash = TSK_NULL,
	}),
);
// clang-format on

// clang-format off
#define TSK_PRIMITIVE_TYPE(type_prefix, type_name, type_parameter)\
	TskOrdering type_prefix##_trait_comparable_compare(const TskType *comparable_type, const TskAny *comparable_1, const TskAny *comparable_2) {\
		(TskEmpty) comparable_type;\
		return (*(const type_name *)comparable_1 > *(const type_name *)comparable_2) - (*(const type_name *)comparable_1 < *(const type_name *)comparable_2);\
	}\
	\
	TSK_TYPE(type_prefix##_type, type_name,\
		TSK_TYPE_TRAIT(type_prefix##_type, TSK_TRAIT_ID_COMPLETE, &(TskTraitComplete){\
			.size = sizeof(type_name),\
			.alignment = alignof(type_name),\
		}),\
		TSK_TYPE_TRAIT(type_prefix##_type, TSK_TRAIT_ID_DROPPABLE, &(TskTraitDroppable){\
			.drop = TSK_NULL,\
		}),\
		TSK_TYPE_TRAIT(type_prefix##_type, TSK_TRAIT_ID_CLONABLE, &(TskTraitClonable){\
			.clone = TSK_NULL,\
		}),\
		TSK_TYPE_TRAIT(type_prefix##_type, TSK_TRAIT_ID_COMPARABLE, &(TskTraitComparable){\
			.compare = type_prefix##_trait_comparable_compare,\
		}),\
		TSK_TYPE_TRAIT(type_prefix##_type, TSK_TRAIT_ID_EQUATABLE, &(TskTraitEquatable){\
			.equals = TSK_NULL,\
		}),\
		TSK_TYPE_TRAIT(type_prefix##_type, TSK_TRAIT_ID_HASHABLE, &(TskTraitHashable){\
			.hash = TSK_NULL,\
		}),\
	)
// clang-format on

TSK_PRIMITIVE_TYPE(tsk_boolean, TskBoolean, boolean);

TSK_PRIMITIVE_TYPE(tsk_character, TskCharacter, character);

TSK_PRIMITIVE_TYPE(tsk_u8, TskU8, u8);
TSK_PRIMITIVE_TYPE(tsk_u16, TskU16, u16);
TSK_PRIMITIVE_TYPE(tsk_u32, TskU32, u32);
TSK_PRIMITIVE_TYPE(tsk_u64, TskU64, u64);
TSK_PRIMITIVE_TYPE(tsk_usize, TskUSize, usize);

TSK_PRIMITIVE_TYPE(tsk_i8, TskI8, i8);
TSK_PRIMITIVE_TYPE(tsk_i16, TskI16, i16);
TSK_PRIMITIVE_TYPE(tsk_i32, TskI32, i32);
TSK_PRIMITIVE_TYPE(tsk_i64, TskI64, i64);
TSK_PRIMITIVE_TYPE(tsk_isize, TskISize, isize);

TSK_PRIMITIVE_TYPE(tsk_f32, TskF32, f32);
TSK_PRIMITIVE_TYPE(tsk_f64, TskF64, f64);
