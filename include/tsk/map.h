#ifndef TSK_MAP_H_INCLUDED
#define TSK_MAP_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <tsk/type.h>
#include <tsk/value.h>

typedef struct TskMapEntry TskMapEntry;
typedef struct TskMap      TskMap;
struct TskMap {
	TskValue       hasher_builder;
	const TskType *key_type;
	const TskType *value_type;
	TskMapEntry   *entries;
	TskAny        *keys;
	TskAny        *values;
	TskUSize       length;
	TskUSize       capacity;
};
TskBool       tsk_map_is_valid(const TskMap *map);
TskMap        tsk_map_new(const TskType *key_type, const TskType *value_type);
TskBool       tsk_map_with_hasher_builder(TskMap *map, const TskType *key_type, const TskType *value_type, const TskType *hasher_builder_type, TskAny *hasher_builder);
TskEmpty      tsk_map_drop(TskMap *map);
TskBool       tsk_map_clone(const TskMap *map_1, TskMap *map_2);
TskUSize      tsk_map_length(const TskMap *map);
TskBool       tsk_map_is_empty(const TskMap *map);
TskUSize      tsk_map_capacity(const TskMap *map);
TskUSize      tsk_map_maximum_capacity(const TskMap *map);
TskF32        tsk_map_load_factor(const TskMap *map);
TskF32        tsk_map_maximum_load_factor(const TskMap *map);
TskAny       *tsk_map_get(TskMap *map, const TskAny *key);
const TskAny *tsk_map_get_const(const TskMap *map, const TskAny *key);
TskEmpty      tsk_map_clear(TskMap *map);
TskBool       tsk_map_reserve(TskMap *map, TskUSize capacity);
TskBool       tsk_map_reserve_additional(TskMap *map, TskUSize additional);
TskBool       tsk_map_insert(TskMap *map, TskAny *key, TskAny *value);
TskAny       *tsk_map_get_or_insert(TskMap *map, TskAny *key, TskAny *value);
TskBool       tsk_map_remove(TskMap *map, const TskAny *key, TskAny *value);
TskBool       tsk_map_equals(const TskMap *map_1, const TskMap *map_2);

extern const TskType tsk_map_type;

typedef struct TskMapIteratorItem TskMapIteratorItem;
struct TskMapIteratorItem {
	const TskAny *key;
	TskAny       *value;
};
typedef struct TskMapIterator TskMapIterator;
struct TskMapIterator {
	TskMap  *map;
	TskUSize index;
};
TskMapIterator tsk_map_iterator(TskMap *map);
TskBool        tsk_map_iterator_is_valid(const TskMapIterator *iterator);
TskBool        tsk_map_iterator_next(TskMapIterator *iterator, TskMapIteratorItem *item);

#ifdef __cplusplus
}
#endif

#endif // TSK_MAP_H_INCLUDED
