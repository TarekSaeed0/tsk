#ifndef TSK_TRAIT_HASHABLE_H_INCLUDED
#define TSK_TRAIT_HASHABLE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <tsk/type.h>

#define TSK_TRAIT_ID_HASHABLE ((TskTraitID)6)
typedef struct TskTraitHashable TskTraitHashable;
struct TskTraitHashable {
	TskEmpty (*hash)(const TskType *hashable_type, const TskAny *hashable, const TskType *hasher_type, TskAny *hasher);
};
TskEmpty tsk_trait_hashable_hash(const TskType *hashable_type, const TskAny *hashable, const TskType *hasher_type, TskAny *hasher);

#ifdef __cplusplus
}
#endif

#endif // TSK_TRAIT_HASHABLE_H_INCLUDED
