#ifndef TSK_MAP_H_INCLUDED
#define TSK_MAP_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <tsk/tuple.h>
#include <tsk/type.h>
#include <tsk/value.h>

typedef struct TskMapEntry TskMapEntry;
typedef struct TskMap      TskMap;
struct TskMap {
	TskValue     hasher_builder;
	TskMapEntry *entries;
	TskAny      *keys;
	TskAny      *values;
	TskUSize     length;
	TskUSize     capacity;
};
TskBoolean     tsk_map_is_valid(const TskType *map_type, const TskMap *map);
TskMap         tsk_map_new(const TskType *map_type);
TskBoolean     tsk_map_with_hasher_builder(const TskType *map_type, TskMap *map, const TskType *hasher_builder_type, TskAny *hasher_builder);
TskEmpty       tsk_map_drop(const TskType *map_type, TskMap *map);
TskBoolean     tsk_map_clone(const TskType *map_type, const TskMap *map_1, TskMap *map_2);
const TskType *tsk_map_key_type(const TskType *map_type);
const TskType *tsk_map_value_type(const TskType *map_type);
TskUSize       tsk_map_length(const TskType *map_type, const TskMap *map);
TskBoolean     tsk_map_is_empty(const TskType *map_type, const TskMap *map);
TskUSize       tsk_map_capacity(const TskType *map_type, const TskMap *map);
TskUSize       tsk_map_maximum_capacity(const TskType *map_type);
TskF32         tsk_map_load_factor(const TskType *map_type, const TskMap *map);
TskF32         tsk_map_maximum_load_factor(const TskType *map_type);
TskAny        *tsk_map_get(const TskType *map_type, TskMap *map, const TskAny *key);
const TskAny  *tsk_map_get_const(const TskType *map_type, const TskMap *map, const TskAny *key);
TskEmpty       tsk_map_clear(const TskType *map_type, TskMap *map);
TskBoolean     tsk_map_reserve(const TskType *map_type, TskMap *map, TskUSize capacity);
TskBoolean     tsk_map_reserve_additional(const TskType *map_type, TskMap *map, TskUSize additional);
TskBoolean     tsk_map_insert(const TskType *map_type, TskMap *map, TskAny *key, TskAny *value);
TskAny        *tsk_map_get_or_insert(const TskType *map_type, TskMap *map, TskAny *key, TskAny *value);
TskBoolean     tsk_map_remove(const TskType *map_type, TskMap *map, const TskAny *key, TskAny *value);
TskBoolean     tsk_map_equals(const TskType *map_type, const TskMap *map_1, const TskMap *map_2);

TskBoolean     tsk_map_type_is_valid(const TskType *map_type);
const TskType *tsk_map_type(const TskType *key_type, const TskType *value_type);

typedef struct TskMapIterator TskMapIterator;
struct TskMapIterator {
	TskMap  *map;
	TskUSize index;
};
TskBoolean     tsk_map_iterator_is_valid(const TskType *iterator_type, const TskMapIterator *iterator);
const TskType *tsk_map_iterator_map_type(const TskType *iterator_type);
const TskType *tsk_map_iterator_item_type(const TskType *iterator_type);
TskBoolean     tsk_map_iterator_next(const TskType *iterator_type, TskMapIterator *iterator, TskTuple *item);

TskBoolean     tsk_map_iterator_type_is_valid(const TskType *iterator_type);
const TskType *tsk_map_iterator_type(const TskType *map_type);

TskMapIterator tsk_map_iterator(const TskType *map_type, TskMap *map);

#ifdef __cplusplus
}
#endif

#endif // TSK_MAP_H_INCLUDED
