#ifndef TSK_TRAIT_HASHER_H_INCLUDED
#define TSK_TRAIT_HASHER_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <tsk/type.h>

#define TSK_TRAIT_ID_HASHER ((TskTraitID)5)
typedef struct TskTraitHasher TskTraitHasher;
struct TskTraitHasher {
	TskEmpty (*combine)(const TskType *hasher_type, TskAny *hasher, const TskU8 *bytes, TskUSize length);
	TskU64 (*finalize)(const TskType *hasher_type, const TskAny *hasher);
};
TskEmpty tsk_trait_hasher_combine(const TskType *hasher_type, TskAny *hasher, const TskU8 *bytes, TskUSize length);
TskU64   tsk_trait_hasher_finalize(const TskType *hasher_type, const TskAny *hasher);

#ifdef __cplusplus
}
#endif

#endif // TSK_TRAIT_HASHER_H_INCLUDED
