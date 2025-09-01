#include <tsk/trait/builder.h>

#include <tsk/trait/complete.h>

#include <assert.h>

const TskType *tsk_trait_builder_built_type(const TskType *builder_type) {

	assert(tsk_type_is_valid(builder_type));

	const TskTraitBuilder *builder_trait = tsk_type_trait(builder_type, TSK_TRAIT_ID_BUILDER);
	assert(builder_trait != TSK_NULL);
	assert(builder_trait->built_type != TSK_NULL);

	const TskType *built_type = builder_trait->built_type(builder_type);
	assert(tsk_type_is_valid(built_type));
	assert(tsk_type_has_trait(built_type, TSK_TRAIT_ID_COMPLETE));

	return built_type;
}
TskEmpty tsk_trait_builder_build(const TskType *builder_type, const TskAny *builder, TskAny *built) {
	assert(tsk_type_is_valid(builder_type));
	assert(builder != TSK_NULL);
	assert(built != TSK_NULL);

	const TskTraitBuilder *builder_trait = tsk_type_trait(builder_type, TSK_TRAIT_ID_BUILDER);
	assert(builder_trait != TSK_NULL);
	assert(builder_trait->build != TSK_NULL);

	builder_trait->build(builder_type, builder, built);
}
