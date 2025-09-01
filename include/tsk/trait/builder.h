#ifndef TSK_TRAIT_BUILDER_H_INCLUDED
#define TSK_TRAIT_BUILDER_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <tsk/type.h>

#define TSK_TRAIT_ID_BUILDER ((TskTraitID)7)
typedef struct TskTraitBuilder TskTraitBuilder;
struct TskTraitBuilder {
	const TskType *(*built_type)(const TskType *builder_type);
	TskEmpty (*build)(const TskType *builder_type, const TskAny *builder, TskAny *built);
};
const TskType *tsk_trait_builder_built_type(const TskType *builder_type);
TskEmpty       tsk_trait_builder_build(const TskType *builder_type, const TskAny *builder, TskAny *built);

#ifdef __cplusplus
}
#endif

#endif // TSK_TRAIT_BUILDER_H_INCLUDED
