#ifndef TSK_TRAIT_ITERABLE_H_INCLUDED
#define TSK_TRAIT_ITERABLE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <tsk/type.h>

#define TSK_TRAIT_ID_ITERABLE ((TskTraitID)9)
typedef struct TskTraitIterable TskTraitIterable;
struct TskTraitIterable {
	const TskType *(*iterator_type)(const TskType *iterable_type);
	TskEmpty (*iterator)(const TskType *iterable_type, const TskAny *iterable, TskAny *iterator);
};
const TskType *tsk_trait_iterable_iterator_type(const TskType *iterable_type);
TskBoolean     tsk_trait_iterable_iterator(const TskType *iterable_type, const TskAny *iterable, TskAny *iterator);

#ifdef __cplusplus
}
#endif

#endif // TSK_TRAIT_ITERABLE_H_INCLUDED
