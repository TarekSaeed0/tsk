#ifndef TSK_LIST_H_INCLUDED
#define TSK_LIST_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <tsk/trait/comparable.h>
#include <tsk/type.h>

typedef struct TskListNode TskListNode;
typedef struct TskList     TskList;
struct TskList {
	TskListNode *head;
	TskListNode *tail;
	TskUSize     length;
};
TskBoolean     tsk_list_is_valid(const TskType *list_type, const TskList *list);
TskList        tsk_list_new(const TskType *list_type);
TskEmpty       tsk_list_drop(const TskType *list_type, TskList *list);
TskBoolean     tsk_list_clone(const TskType *list_type, const TskList *list_1, TskList *list_2);
const TskType *tsk_list_element_type(const TskType *list_type);
TskUSize       tsk_list_length(const TskType *list_type, const TskList *list);
TskBoolean     tsk_list_is_empty(const TskType *list_type, const TskList *list);
TskAny        *tsk_list_front(const TskType *list_type, TskList *list);
const TskAny  *tsk_list_front_const(const TskType *list_type, const TskList *list);
TskAny        *tsk_list_back(const TskType *list_type, TskList *list);
const TskAny  *tsk_list_back_const(const TskType *list_type, const TskList *list);
TskEmpty       tsk_list_clear(const TskType *list_type, TskList *list);
TskBoolean     tsk_list_push_front(const TskType *list_type, TskList *list, TskAny *element);
TskBoolean     tsk_list_push_back(const TskType *list_type, TskList *list, TskAny *element);
TskBoolean     tsk_list_pop_front(const TskType *list_type, TskList *list, TskAny *element);
TskBoolean     tsk_list_pop_back(const TskType *list_type, TskList *list, TskAny *element);
TskOrdering    tsk_list_compare(const TskType *list_type, const TskList *list_1, const TskList *list_2);
TskBoolean     tsk_list_equals(const TskType *list_type, const TskList *list_1, const TskList *list_2);
TskEmpty       tsk_list_hash(const TskType *list_type, const TskList *list, const TskType *hasher_type, TskAny *hasher);

TskBoolean     tsk_list_type_is_valid(const TskType *list_type);
const TskType *tsk_list_type(const TskType *element_type);

typedef struct TskListIterator TskListIterator;
struct TskListIterator {
	TskList     *list;
	TskListNode *current;
};
TskBoolean     tsk_list_iterator_is_valid(const TskType *list_iterator_type, const TskListIterator *list_iterator);
const TskType *tsk_list_iterator_element_type(const TskType *list_iterator_type);
const TskType *tsk_list_iterator_item_type(const TskType *list_iterator_type);
TskBoolean     tsk_list_iterator_next(const TskType *list_iterator_type, TskListIterator *list_iterator, TskAny *item);

TskBoolean     tsk_list_iterator_type_is_valid(const TskType *list_iterator_type);
const TskType *tsk_list_iterator_type(const TskType *element_type);

TskListIterator tsk_list_iterator(const TskType *list_type, TskList *list);

typedef struct TskListIteratorConst TskListIteratorConst;
struct TskListIteratorConst {
	const TskList     *list;
	const TskListNode *current;
};
TskBoolean     tsk_list_iterator_const_is_valid(const TskType *list_iterator_type, const TskListIteratorConst *list_iterator);
const TskType *tsk_list_iterator_const_element_type(const TskType *list_iterator_type);
const TskType *tsk_list_iterator_const_item_type(const TskType *list_iterator_type);
TskBoolean     tsk_list_iterator_const_next(const TskType *list_iterator_type, TskListIteratorConst *list_iterator, TskAny *item);

TskBoolean     tsk_list_iterator_const_type_is_valid(const TskType *list_iterator_type);
const TskType *tsk_list_iterator_const_type(const TskType *element_type);

TskListIteratorConst tsk_list_iterator_const(const TskType *list_type, const TskList *list);

#ifdef __cplusplus
}
#endif

#endif // TSK_LIST_H_INCLUDED
