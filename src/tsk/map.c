#include <tsk/map.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef enum TskMapEntryState {
	TSK_MAP_ENTRY_STATE_EMPTY,
	TSK_MAP_ENTRY_STATE_OCCUPIED,
	TSK_MAP_ENTRY_STATE_DELETED,
} TskMapEntryState;

struct TskMapEntry {
	TskU64           hash;
	TskMapEntryState state;
};

static inline TskAny *tsk_map_get_key(TskMap *map, TskUSize index) {
	assert(tsk_map_is_valid(map));
	assert(index < tsk_map_capacity(map));

	return (TskU8 *)map->keys + (index * tsk_trait_complete_size(tsk_type_trait(map->key_type, TSK_TRAIT_COMPLETE)));
}
static inline const TskAny *tsk_map_get_key_const(const TskMap *map, TskUSize index) {
	assert(tsk_map_is_valid(map));
	assert(index < tsk_map_capacity(map));

	return (const TskU8 *)map->keys + (index * tsk_trait_complete_size(tsk_type_trait(map->key_type, TSK_TRAIT_COMPLETE)));
}
static inline TskAny *tsk_map_get_value(TskMap *map, TskUSize index) {
	assert(tsk_map_is_valid(map));
	assert(index < tsk_map_capacity(map));

	return (TskU8 *)map->values + (index * tsk_trait_complete_size(tsk_type_trait(map->value_type, TSK_TRAIT_COMPLETE)));
}
static inline const TskAny *tsk_map_get_value_const(const TskMap *map, TskUSize index) {
	assert(tsk_map_is_valid(map));
	assert(index < tsk_map_capacity(map));

	return (const TskU8 *)map->values + (index * tsk_trait_complete_size(tsk_type_trait(map->value_type, TSK_TRAIT_COMPLETE)));
}
static inline TskU64 tsk_map_hash_key(const TskMap *map, const TskAny *key) {
	assert(tsk_map_is_valid(map));
	assert(key != TSK_NULL);

	TskValue hasher;
	tsk_value_new(
	    &hasher,
	    tsk_trait_builder_built_type(tsk_type_trait(tsk_value_type(&map->hasher_builder), TSK_TRAIT_BUILDER))
	);
	tsk_trait_builder_build(
	    tsk_type_trait(tsk_value_type(&map->hasher_builder), TSK_TRAIT_BUILDER),
	    tsk_value_data_const(&map->hasher_builder),
	    tsk_value_data(&hasher)
	);

	tsk_trait_hashable_hash(
	    tsk_type_trait(map->key_type, TSK_TRAIT_HASHABLE),
	    key,
	    tsk_type_trait(&tsk_default_hasher_type, TSK_TRAIT_HASHER),
	    &hasher
	);

	TskU64 hash = tsk_default_hasher_finalize(tsk_value_data_const(&hasher));

	tsk_value_drop(&hasher);

	return hash;
}

TskBool tsk_map_is_valid(const TskMap *map) {
	return map != NULL &&
	       tsk_type_is_valid(map->key_type) && tsk_type_has_trait(map->key_type, TSK_TRAIT_COMPLETE) &&
	       tsk_type_has_trait(map->key_type, TSK_TRAIT_HASHABLE) && tsk_type_has_trait(map->key_type, TSK_TRAIT_EQUATABLE) &&
	       tsk_type_is_valid(map->value_type) && tsk_type_has_trait(map->value_type, TSK_TRAIT_COMPLETE) &&
	       ((map->entries != TSK_NULL && map->keys != TSK_NULL && map->values != TSK_NULL) || map->capacity == 0) && map->length <= map->capacity;
}
TskMap tsk_map_new(const TskType *key_type, const TskType *value_type) {
	assert(tsk_type_is_valid(key_type));
	assert(tsk_type_has_trait(key_type, TSK_TRAIT_COMPLETE));
	assert(tsk_type_has_trait(key_type, TSK_TRAIT_HASHABLE));
	assert(tsk_type_has_trait(key_type, TSK_TRAIT_EQUATABLE));
	assert(tsk_type_is_valid(value_type));
	assert(tsk_type_has_trait(value_type, TSK_TRAIT_COMPLETE));

	TskMap map;

	TskBool success = tsk_map_with_hasher_builder(&map, key_type, value_type, &tsk_default_hasher_builder_type, &tsk_default_hasher_builder);
	assert(success);
	(void)success;

	assert(tsk_map_is_valid(&map));

	return map;
}
TskBool tsk_map_with_hasher_builder(TskMap *map, const TskType *key_type, const TskType *value_type, const TskType *hasher_builder_type, TskAny *hasher_builder) {
	assert(map != TSK_NULL);
	assert(tsk_type_is_valid(key_type));
	assert(tsk_type_has_trait(key_type, TSK_TRAIT_COMPLETE));
	assert(tsk_type_has_trait(key_type, TSK_TRAIT_HASHABLE));
	assert(tsk_type_has_trait(key_type, TSK_TRAIT_EQUATABLE));
	assert(tsk_type_is_valid(value_type));
	assert(tsk_type_has_trait(value_type, TSK_TRAIT_COMPLETE));
	assert(tsk_type_is_valid(hasher_builder_type));
	assert(tsk_type_has_trait(hasher_builder_type, TSK_TRAIT_COMPLETE));
	assert(tsk_type_has_trait(hasher_builder_type, TSK_TRAIT_BUILDER));
	assert(tsk_type_has_trait(tsk_trait_builder_built_type(tsk_type_trait(hasher_builder_type, TSK_TRAIT_BUILDER)), TSK_TRAIT_HASHER));
	assert(hasher_builder != TSK_NULL);

	if (!tsk_value_new(&map->hasher_builder, hasher_builder_type)) {
		return TSK_FALSE;
	}

	memcpy(
	    tsk_value_data(&map->hasher_builder),
	    hasher_builder,
	    tsk_trait_complete_size(tsk_type_trait(hasher_builder_type, TSK_TRAIT_COMPLETE))
	);

	map->key_type   = key_type;
	map->value_type = value_type;
	map->entries    = TSK_NULL;
	map->keys       = TSK_NULL;
	map->values     = TSK_NULL;
	map->length     = 0;
	map->capacity   = 0;

	assert(tsk_map_is_valid(map));

	return TSK_TRUE;
}
TskEmpty tsk_map_drop(TskMap *map) {
	assert(tsk_map_is_valid(map));

	tsk_map_clear(map);

	tsk_value_drop(&map->hasher_builder);

	free(map->entries);
	if (tsk_trait_complete_size(tsk_type_trait(map->key_type, TSK_TRAIT_COMPLETE)) != 0) {
		free(map->keys);
	}
	if (tsk_trait_complete_size(tsk_type_trait(map->value_type, TSK_TRAIT_COMPLETE)) != 0) {
		free(map->values);
	}

	map->entries  = TSK_NULL;
	map->keys     = TSK_NULL;
	map->values   = TSK_NULL;
	map->capacity = 0;
}
TskBool tsk_map_clone(const TskMap *map_1, TskMap *map_2) {
	assert(tsk_map_is_valid(map_1));
	assert(map_2 != TSK_NULL);

	if (tsk_map_is_empty(map_1)) {
		*map_2 = tsk_map_new(map_1->key_type, map_1->value_type);
		return TSK_TRUE;
	}

	TskMap map = tsk_map_new(map_1->key_type, map_1->value_type);
	if (!tsk_map_reserve_additional(&map, tsk_map_length(map_1))) {
		return TSK_FALSE;
	}

	for (TskUSize i = 0; i < tsk_map_capacity(map_1); i++) {
		if (map_1->entries[i].state == TSK_MAP_ENTRY_STATE_OCCUPIED) {
			TskUSize index = map_1->entries[i].hash % tsk_map_capacity(&map);
			while (map.entries[index].state == TSK_MAP_ENTRY_STATE_OCCUPIED) {
				index = (index + 1) % tsk_map_capacity(&map);
			}

			if (!tsk_trait_clonable_clone(
			        tsk_type_trait(map_1->key_type, TSK_TRAIT_COMPLETE),
			        tsk_map_get_key_const(map_1, i),
			        tsk_map_get_key(&map, index)
			    )) {
				tsk_map_drop(&map);
				return TSK_FALSE;
			}

			if (!tsk_trait_clonable_clone(
			        tsk_type_trait(map_1->value_type, TSK_TRAIT_COMPLETE),
			        tsk_map_get_value_const(map_1, i),
			        tsk_map_get_value(&map, index)
			    )) {
				tsk_trait_droppable_drop(
				    tsk_type_trait(map_1->key_type, TSK_TRAIT_DROPPABLE),
				    tsk_map_get_key(&map, index)
				);
				tsk_map_drop(&map);
				return TSK_FALSE;
			}

			map.entries[index] = map_1->entries[i];
		}
	}

	map.length = map_1->length;

	*map_2     = map;

	return TSK_TRUE;
}
TskUSize tsk_map_length(const TskMap *map) {
	assert(tsk_map_is_valid(map));

	return map->length;
}
TskBool tsk_map_is_empty(const TskMap *map) {
	assert(tsk_map_is_valid(map));

	return tsk_map_length(map) == 0;
}
TskUSize tsk_map_capacity(const TskMap *map) {
	assert(tsk_map_is_valid(map));

	return map->capacity;
}
TskUSize tsk_map_maximum_capacity(const TskMap *map) {
	TskUSize entry_size   = sizeof(TskMapEntry);
	TskUSize key_size     = tsk_trait_complete_size(tsk_type_trait(map->key_type, TSK_TRAIT_COMPLETE));
	TskUSize value_size   = tsk_trait_complete_size(tsk_type_trait(map->value_type, TSK_TRAIT_COMPLETE));

	TskUSize maximum_size = entry_size > key_size ? entry_size : key_size;
	maximum_size          = maximum_size > value_size ? maximum_size : value_size;

	return SIZE_MAX / maximum_size;
}
TskF32 tsk_map_load_factor(const TskMap *map) {
	assert(tsk_map_is_valid(map));

	if (map->capacity == 0) {
		return 0.0F;
	}

	return (TskF32)map->length / (TskF32)map->capacity;
}
TskF32 tsk_map_maximum_load_factor(const TskMap *map) {
	assert(tsk_map_is_valid(map));

	return 0.5F;
}
TskAny *tsk_map_get(TskMap *map, const TskAny *key) {
	assert(tsk_map_is_valid(map));
	assert(key != TSK_NULL);

	if (tsk_map_is_empty(map)) {
		return TSK_NULL;
	}

	TskU64   hash           = tsk_map_hash_key(map, key);
	TskUSize starting_index = hash % tsk_map_capacity(map);
	TskUSize index          = starting_index;
	while (map->entries[index].state != TSK_MAP_ENTRY_STATE_EMPTY) {
		if (map->entries[index].state == TSK_MAP_ENTRY_STATE_OCCUPIED &&
		    map->entries[index].hash == hash &&
		    tsk_trait_equatable_equals(
		        tsk_type_trait(map->key_type, TSK_TRAIT_EQUATABLE),
		        tsk_map_get_key_const(map, index),
		        key
		    )) {
			return tsk_map_get_value(map, index);
		}
		index = (index + 1) % tsk_map_capacity(map);
		if (index == starting_index) {
			break;
		}
	}

	return TSK_NULL;
}
const TskAny *tsk_map_get_const(const TskMap *map, const TskAny *key) {
	assert(tsk_map_is_valid(map));
	assert(key != TSK_NULL);

	if (tsk_map_is_empty(map)) {
		return TSK_NULL;
	}

	TskU64   hash           = tsk_map_hash_key(map, key);
	TskUSize starting_index = hash % tsk_map_capacity(map);
	TskUSize index          = starting_index;
	while (map->entries[index].state != TSK_MAP_ENTRY_STATE_EMPTY) {
		if (map->entries[index].state == TSK_MAP_ENTRY_STATE_OCCUPIED &&
		    map->entries[index].hash == hash &&
		    tsk_trait_equatable_equals(
		        tsk_type_trait(map->key_type, TSK_TRAIT_EQUATABLE),
		        tsk_map_get_key_const(map, index),
		        key
		    )) {
			return tsk_map_get_value_const(map, index);
		}
		index = (index + 1) % tsk_map_capacity(map);
		if (index == starting_index) {
			break;
		}
	}

	return TSK_NULL;
}
TskEmpty tsk_map_clear(TskMap *map) {
	assert(tsk_map_is_valid(map));
	assert(tsk_type_has_trait(map->key_type, TSK_TRAIT_DROPPABLE));
	assert(tsk_type_has_trait(map->value_type, TSK_TRAIT_DROPPABLE));

	for (TskUSize i = 0; i < tsk_map_capacity(map); i++) {
		if (map->entries[i].state == TSK_MAP_ENTRY_STATE_OCCUPIED) {
			tsk_trait_droppable_drop(
			    tsk_type_trait(map->key_type, TSK_TRAIT_DROPPABLE),
			    tsk_map_get_key(map, i)
			);
			tsk_trait_droppable_drop(
			    tsk_type_trait(map->value_type, TSK_TRAIT_DROPPABLE),
			    tsk_map_get_value(map, i)
			);
		}
	}

	map->length = 0;
}
TskBool tsk_map_reserve(TskMap *map, TskUSize capacity) {
	assert(tsk_map_is_valid(map));

	if (capacity <= tsk_map_capacity(map)) {
		return TSK_TRUE;
	}

	if (capacity > tsk_map_maximum_capacity(map)) {
		return TSK_FALSE;
	}

	TskMapEntry *entries = calloc(capacity, sizeof(TskMapEntry));
	if (entries == TSK_NULL) {
		return TSK_FALSE;
	}

	TskAny *keys = TSK_NULL;
	if (tsk_trait_complete_size(tsk_type_trait(map->key_type, TSK_TRAIT_COMPLETE)) == 0) {
		keys = (TskAny *)tsk_trait_complete_alignment(tsk_type_trait(map->key_type, TSK_TRAIT_COMPLETE)); // NOLINT(performance-no-int-to-ptr)
	} else {
		keys = malloc(capacity * tsk_trait_complete_size(tsk_type_trait(map->key_type, TSK_TRAIT_COMPLETE)));
		if (keys == TSK_NULL) {
			free(entries);
			return TSK_FALSE;
		}
	}

	TskAny *values = TSK_NULL;
	if (tsk_trait_complete_size(tsk_type_trait(map->value_type, TSK_TRAIT_COMPLETE)) == 0) {
		values = (TskAny *)tsk_trait_complete_alignment(tsk_type_trait(map->value_type, TSK_TRAIT_COMPLETE)); // NOLINT(performance-no-int-to-ptr)
	} else {
		values = malloc(capacity * tsk_trait_complete_size(tsk_type_trait(map->value_type, TSK_TRAIT_COMPLETE)));
		if (values == TSK_NULL) {
			free(entries);
			if (tsk_trait_complete_size(tsk_type_trait(map->key_type, TSK_TRAIT_COMPLETE)) != 0) {
				free(keys);
			}
			return TSK_FALSE;
		}
	}

	for (TskUSize i = 0; i < tsk_map_capacity(map); i++) {
		if (map->entries[i].state == TSK_MAP_ENTRY_STATE_OCCUPIED) {
			TskUSize index = map->entries[i].hash % capacity;
			while (entries[index].state == TSK_MAP_ENTRY_STATE_OCCUPIED) {
				index = (index + 1) % capacity;
			}

			entries[index] = map->entries[i];
			memcpy(
			    (TskU8 *)keys + (index * tsk_trait_complete_size(tsk_type_trait(map->key_type, TSK_TRAIT_COMPLETE))),
			    tsk_map_get_key_const(map, i),
			    tsk_trait_complete_size(tsk_type_trait(map->key_type, TSK_TRAIT_COMPLETE))
			);
			memcpy(
			    (TskU8 *)values + (index * tsk_trait_complete_size(tsk_type_trait(map->value_type, TSK_TRAIT_COMPLETE))),
			    tsk_map_get_value_const(map, i),
			    tsk_trait_complete_size(tsk_type_trait(map->value_type, TSK_TRAIT_COMPLETE))
			);
		}
	}

	free(map->entries);
	if (tsk_trait_complete_size(tsk_type_trait(map->key_type, TSK_TRAIT_COMPLETE)) != 0) {
		free(map->keys);
	}
	if (tsk_trait_complete_size(tsk_type_trait(map->value_type, TSK_TRAIT_COMPLETE)) != 0) {
		free(map->values);
	}

	map->entries  = entries;
	map->keys     = keys;
	map->values   = values;
	map->capacity = capacity;

	return TSK_TRUE;
}
TskBool tsk_map_reserve_additional(TskMap *map, TskUSize additional) {
	assert(tsk_map_is_valid(map));

	TskUSize maximum_capacity = tsk_map_maximum_capacity(map);
	if (additional > maximum_capacity - tsk_map_length(map)) {
		return TSK_FALSE;
	}

	if ((TskF32)tsk_map_capacity(map) >= (TskF32)(tsk_map_length(map) + additional) / tsk_map_maximum_load_factor(map)) {
		assert(tsk_map_capacity(map) >= tsk_map_length(map) + additional);
		return TSK_TRUE;
	}

	TskUSize capacity = tsk_map_capacity(map);
	do {
		if (capacity == 0) {
			capacity = 1;
		} else if (capacity > maximum_capacity / 2) {
			capacity = maximum_capacity;
			break;
		} else {
			capacity *= 2;
		}
	} while ((TskF32)capacity < (TskF32)(tsk_map_length(map) + additional) / tsk_map_maximum_load_factor(map));
	assert(capacity >= tsk_map_length(map) + additional);

	return tsk_map_reserve(map, capacity);
}
TskBool tsk_map_insert(TskMap *map, TskAny *key, TskAny *value) {
	assert(tsk_map_is_valid(map));
	assert(key != TSK_NULL);
	assert(value != TSK_NULL);

	if (!tsk_map_reserve_additional(map, 1)) {
		return TSK_FALSE;
	}

	TskU64   hash           = tsk_map_hash_key(map, key);
	TskUSize starting_index = hash % tsk_map_capacity(map);
	TskUSize index          = starting_index;
	while (map->entries[index].state == TSK_MAP_ENTRY_STATE_OCCUPIED) {
		if (map->entries[index].hash == hash &&
		    tsk_trait_equatable_equals(
		        tsk_type_trait(map->key_type, TSK_TRAIT_EQUATABLE),
		        tsk_map_get_key_const(map, index),
		        key
		    )) {
			assert(tsk_type_has_trait(map->key_type, TSK_TRAIT_DROPPABLE));
			assert(tsk_type_has_trait(map->value_type, TSK_TRAIT_DROPPABLE));

			tsk_trait_droppable_drop(
			    tsk_type_trait(map->key_type, TSK_TRAIT_DROPPABLE),
			    key
			);
			tsk_trait_droppable_drop(
			    tsk_type_trait(map->value_type, TSK_TRAIT_DROPPABLE),
			    tsk_map_get_value(map, index)
			);
			memcpy(
			    tsk_map_get_value(map, index),
			    value,
			    tsk_trait_complete_size(tsk_type_trait(map->value_type, TSK_TRAIT_COMPLETE))
			);

			return TSK_TRUE;
		}
		index = (index + 1) % tsk_map_capacity(map);
		if (index == starting_index) {
			return TSK_FALSE;
		}
	}

	map->entries[index].hash  = hash;
	map->entries[index].state = TSK_MAP_ENTRY_STATE_OCCUPIED;

	memcpy(
	    tsk_map_get_key(map, index),
	    key,
	    tsk_trait_complete_size(tsk_type_trait(map->key_type, TSK_TRAIT_COMPLETE))
	);
	memcpy(
	    tsk_map_get_value(map, index),
	    value,
	    tsk_trait_complete_size(tsk_type_trait(map->value_type, TSK_TRAIT_COMPLETE))
	);

	map->length++;

	return TSK_TRUE;
}
TskAny *tsk_map_get_or_insert(TskMap *map, TskAny *key, TskAny *value) {
	assert(tsk_map_is_valid(map));
	assert(key != TSK_NULL);
	assert(value != TSK_NULL);

	if (!tsk_map_reserve_additional(map, 1)) {
		return TSK_NULL;
	}

	TskU64   hash           = tsk_map_hash_key(map, key);
	TskUSize starting_index = hash % tsk_map_capacity(map);
	TskUSize index          = starting_index;
	while (map->entries[index].state == TSK_MAP_ENTRY_STATE_OCCUPIED) {
		if (map->entries[index].hash == hash &&
		    tsk_trait_equatable_equals(
		        tsk_type_trait(map->key_type, TSK_TRAIT_EQUATABLE),
		        tsk_map_get_key_const(map, index),
		        key
		    )) {
			assert(tsk_type_has_trait(map->key_type, TSK_TRAIT_DROPPABLE));
			assert(tsk_type_has_trait(map->value_type, TSK_TRAIT_DROPPABLE));

			tsk_trait_droppable_drop(
			    tsk_type_trait(map->key_type, TSK_TRAIT_DROPPABLE),
			    key
			);
			tsk_trait_droppable_drop(
			    tsk_type_trait(map->value_type, TSK_TRAIT_DROPPABLE),
			    value
			);

			return tsk_map_get_value(map, index);
		}
		index = (index + 1) % tsk_map_capacity(map);
		if (index == starting_index) {
			return TSK_FALSE;
		}
	}

	map->entries[index].hash  = hash;
	map->entries[index].state = TSK_MAP_ENTRY_STATE_OCCUPIED;

	memcpy(
	    tsk_map_get_key(map, index),
	    key,
	    tsk_trait_complete_size(tsk_type_trait(map->key_type, TSK_TRAIT_COMPLETE))
	);
	memcpy(
	    tsk_map_get_value(map, index),
	    value,
	    tsk_trait_complete_size(tsk_type_trait(map->value_type, TSK_TRAIT_COMPLETE))
	);

	map->length++;

	return tsk_map_get_value(map, index);
}
TskBool tsk_map_remove(TskMap *map, const TskAny *key, TskAny *value) {
	assert(tsk_map_is_valid(map));
	assert(key != TSK_NULL);
	assert(tsk_type_has_trait(map->key_type, TSK_TRAIT_DROPPABLE));

	if (tsk_map_is_empty(map)) {
		return TSK_FALSE;
	}

	TskU64   hash           = tsk_map_hash_key(map, key);
	TskUSize starting_index = hash % tsk_map_capacity(map);
	TskUSize index          = starting_index;
	while (map->entries[index].state != TSK_MAP_ENTRY_STATE_EMPTY) {
		if (map->entries[index].state == TSK_MAP_ENTRY_STATE_OCCUPIED &&
		    map->entries[index].hash == hash &&
		    tsk_trait_equatable_equals(
		        tsk_type_trait(map->key_type, TSK_TRAIT_EQUATABLE),
		        tsk_map_get_key_const(map, index),
		        key
		    )) {
			map->entries[index].state = TSK_MAP_ENTRY_STATE_DELETED;

			tsk_trait_droppable_drop(
			    tsk_type_trait(map->key_type, TSK_TRAIT_DROPPABLE),
			    tsk_map_get_key(map, index)
			);

			if (value != TSK_NULL) {
				memcpy(
				    value,
				    tsk_map_get_value_const(map, index),
				    tsk_trait_complete_size(tsk_type_trait(map->value_type, TSK_TRAIT_COMPLETE))
				);
			} else {
				assert(tsk_type_has_trait(map->value_type, TSK_TRAIT_DROPPABLE));
				tsk_trait_droppable_drop(
				    tsk_type_trait(map->value_type, TSK_TRAIT_DROPPABLE),
				    tsk_map_get_value(map, index)
				);
			}

			map->length--;

			return TSK_TRUE;
		}
		index = (index + 1) % tsk_map_capacity(map);
		if (index == starting_index) {
			break;
		}
	}

	return TSK_FALSE;
}
TskBool tsk_map_equals(const TskMap *map_1, const TskMap *map_2) {
	assert(tsk_map_is_valid(map_1));
	assert(tsk_map_is_valid(map_2));
	assert(map_1->key_type == map_2->key_type);
	assert(map_1->value_type == map_2->value_type);
	assert(tsk_type_has_trait(map_1->value_type, TSK_TRAIT_EQUATABLE));

	if (tsk_map_length(map_1) != tsk_map_length(map_2)) {
		return TSK_FALSE;
	}

	for (TskUSize i = 0; i < tsk_map_capacity(map_1); i++) {
		if (map_1->entries[i].state == TSK_MAP_ENTRY_STATE_OCCUPIED) {
			const TskAny *key_1   = tsk_map_get_key_const(map_1, i);
			const TskAny *value_1 = tsk_map_get_value_const(map_1, i);

			const TskAny *value_2 = tsk_map_get_const(map_2, key_1);
			if (value_2 == TSK_NULL) {
				return TSK_FALSE;
			}

			if (!tsk_trait_equatable_equals(
			        tsk_type_trait(map_1->value_type, TSK_TRAIT_EQUATABLE),
			        value_1,
			        value_2
			    )) {
				return TSK_FALSE;
			}
		}
	}

	return TSK_TRUE;
}

static const TskTraitComplete tsk_map_trait_complete = {
	.size      = sizeof(TskMap),
	.alignment = alignof(TskMap),
};

static TskEmpty tsk_map_trait_droppable_drop(TskAny *map) {
	tsk_map_drop(map);
}
static const TskTraitDroppable tsk_map_trait_droppable = {
	.drop = tsk_map_trait_droppable_drop,
};

static TskBool tsk_map_trait_clonable_clone(const TskAny *map_1, TskAny *map_2) {
	return tsk_map_clone(map_1, map_2);
}
static const TskTraitClonable tsk_map_trait_clonable = {
	.trait_complete = &tsk_map_trait_complete,
	.clone          = tsk_map_trait_clonable_clone,
};

static TskBool tsk_map_type_equatable_equals(const TskAny *map_1, const TskAny *map_2) {
	return tsk_map_equals(map_1, map_2);
}
static const TskTraitEquatable tsk_map_trait_equatable = {
	.equals = tsk_map_type_equatable_equals,
};

const TskType tsk_map_type = {
	.name   = "TskMap",
	.traits = {
	    [TSK_TRAIT_COMPLETE]  = &tsk_map_trait_complete,
	    [TSK_TRAIT_DROPPABLE] = &tsk_map_trait_droppable,
	    [TSK_TRAIT_CLONABLE]  = &tsk_map_trait_clonable,
	    [TSK_TRAIT_EQUATABLE] = &tsk_map_trait_equatable,
	},
};

TskMapIterator tsk_map_iterator(TskMap *map) {
	assert(tsk_map_is_valid(map));

	TskMapIterator iterator = {
		.map   = map,
		.index = 0,
	};

	assert(tsk_map_iterator_is_valid(&iterator));

	return iterator;
}
TskBool tsk_map_iterator_is_valid(const TskMapIterator *iterator) {
	return iterator != TSK_NULL &&
	       tsk_map_is_valid(iterator->map) &&
	       iterator->index <= tsk_map_capacity(iterator->map);
}
TskBool tsk_map_iterator_next(TskMapIterator *iterator, TskMapIteratorItem *item) {
	assert(tsk_map_iterator_is_valid(iterator));
	assert(item != TSK_NULL);

	while (iterator->index < tsk_map_capacity(iterator->map)) {
		if (iterator->map->entries[iterator->index].state == TSK_MAP_ENTRY_STATE_OCCUPIED) {
			item->key   = tsk_map_get_key_const(iterator->map, iterator->index);
			item->value = tsk_map_get_value(iterator->map, iterator->index);
			iterator->index++;
			return TSK_TRUE;
		}
		iterator->index++;
	}

	return TSK_FALSE;
}
