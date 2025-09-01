#ifndef TSK_TRAIT_ITERATOR_H_INCLUDED
#define TSK_TRAIT_ITERATOR_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <tsk/type.h>

#define TSK_TRAIT_ID_ITERATOR ((TskTraitID)8)
typedef struct TskTraitIterator TskTraitIterator;
struct TskTraitIterator {
	const TskType *(*item_type)(const TskType *iterator_type);
	TskBoolean (*next)(const TskType *iterator_type, TskAny *iterator, TskAny *item);
};
const TskType *tsk_trait_iterator_item_type(const TskType *iterator_type);
TskBoolean     tsk_trait_iterator_next(const TskType *iterator_type, TskAny *iterator, TskAny *item);

#ifdef __cplusplus
}
#endif

#endif // TSK_TRAIT_ITERATOR_H_INCLUDED
