#ifndef TSK_LIST_H_INCLUDED
#define TSK_LIST_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <tsk/type.h>

typedef struct TskListNode TskListNode;
typedef struct TskList     TskList;
struct TskList {
	const TskType *element_type;
	TskListNode   *head;
	TskListNode   *tail;
	TskUSize       length;
};
TskBool       tsk_list_is_valid(const TskList *list);
TskList       tsk_list_new(const TskType *element_type);
TskEmpty      tsk_list_drop(TskList *list);
TskBool       tsk_list_clone(const TskList *list_1, TskList *list_2);
TskUSize      tsk_list_length(const TskList *list);
TskBool       tsk_list_is_empty(const TskList *list);
TskAny       *tsk_list_front(TskList *list);
const TskAny *tsk_list_front_const(const TskList *list);
TskAny       *tsk_list_back(TskList *list);
const TskAny *tsk_list_back_const(const TskList *list);
TskEmpty      tsk_list_clear(TskList *list);
TskBool       tsk_list_push_front(TskList *list, TskAny *element);
TskBool       tsk_list_push_back(TskList *list, TskAny *element);
TskBool       tsk_list_pop_front(TskList *list, TskAny *element);
TskBool       tsk_list_pop_back(TskList *list, TskAny *element);
TskOrdering   tsk_list_compare(const TskList *list_1, const TskList *list_2);
TskBool       tsk_list_equals(const TskList *list_1, const TskList *list_2);
TskEmpty      tsk_list_hash(const TskList *list, const TskTraitHasher *trait_hasher, TskAny *hasher);

extern const TskType tsk_list_type;

#ifdef __cplusplus
}
#endif

#endif // TSK_LIST_H_INCLUDED
