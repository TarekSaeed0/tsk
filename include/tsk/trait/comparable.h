#ifndef TSK_TRAIT_COMPARABLE_H_INCLUDED
#define TSK_TRAIT_COMPARABLE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <tsk/type.h>

typedef enum TskOrdering {
	TSK_ORDERING_LESS    = -1,
	TSK_ORDERING_EQUAL   = 0,
	TSK_ORDERING_GREATER = 1
} TskOrdering;

#define TSK_TRAIT_ID_COMPARABLE ((TskTraitID)3)
typedef struct TskTraitComparable TskTraitComparable;
struct TskTraitComparable {
	TskOrdering (*compare)(const TskType *comparable_type, const TskAny *comparable_1, const TskAny *comparable_2);
};
TskOrdering tsk_trait_comparable_compare(const TskType *comparable_type, const TskAny *comparable_1, const TskAny *comparable_2);

#ifdef __cplusplus
}
#endif

#endif // TSK_TRAIT_COMPARABLE_H_INCLUDED
