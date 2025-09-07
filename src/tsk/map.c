#include <tsk/map.h>

#include <tsk/default_hasher.h>
#include <tsk/reference.h>
#include <tsk/trait/builder.h>
#include <tsk/trait/clonable.h>
#include <tsk/trait/comparable.h>
#include <tsk/trait/complete.h>
#include <tsk/trait/droppable.h>
#include <tsk/trait/equatable.h>
#include <tsk/trait/hashable.h>
#include <tsk/trait/hasher.h>
#include <tsk/trait/iterable.h>
#include <tsk/trait/iterator.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct TskMapType TskMapType;
struct TskMapType {
	TskType                map_type;
	TskCharacter           map_type_name[40];
	TskTypeTraitTable      map_type_trait_table;
	TskTypeTraitTableEntry map_type_trait_table_entries[16];
	const TskType         *key_type;
	const TskType         *value_type;
};

typedef enum TskMapEntryState {
	TSK_MAP_ENTRY_STATE_EMPTY,
	TSK_MAP_ENTRY_STATE_OCCUPIED,
	TSK_MAP_ENTRY_STATE_DELETED,
} TskMapEntryState;

struct TskMapEntry {
	TskU64           hash;
	TskMapEntryState state;
};

static inline TskAny *tsk_map_get_key(const TskType *map_type, TskMap *map, TskUSize index) {
	assert(tsk_map_type_is_valid(map_type));
	assert(tsk_map_is_valid(map_type, map));
	assert(index < tsk_map_capacity(map_type, map));

	return (TskU8 *)map->keys + (index * tsk_trait_complete_size(tsk_map_key_type(map_type)));
}
static inline const TskAny *tsk_map_get_key_const(const TskType *map_type, const TskMap *map, TskUSize index) {
	assert(tsk_map_type_is_valid(map_type));
	assert(tsk_map_is_valid(map_type, map));
	assert(index < tsk_map_capacity(map_type, map));

	return (const TskU8 *)map->keys + (index * tsk_trait_complete_size(tsk_map_key_type(map_type)));
}
static inline TskAny *tsk_map_get_value(const TskType *map_type, TskMap *map, TskUSize index) {
	assert(tsk_map_type_is_valid(map_type));
	assert(tsk_map_is_valid(map_type, map));
	assert(index < tsk_map_capacity(map_type, map));

	return (TskU8 *)map->values + (index * tsk_trait_complete_size(tsk_map_value_type(map_type)));
}
static inline const TskAny *tsk_map_get_value_const(const TskType *map_type, const TskMap *map, TskUSize index) {
	assert(tsk_map_type_is_valid(map_type));
	assert(tsk_map_is_valid(map_type, map));
	assert(index < tsk_map_capacity(map_type, map));

	return (const TskU8 *)map->values + (index * tsk_trait_complete_size(tsk_map_value_type(map_type)));
}
static inline TskU64 tsk_map_hash_key(const TskType *map_type, const TskMap *map, const TskAny *key) {
	assert(tsk_map_type_is_valid(map_type));
	assert(tsk_map_is_valid(map_type, map));
	assert(key != TSK_NULL);

	const TskType *hasher_builder_type = TSK_NULL;
	const TskAny  *hasher_builder      = TSK_NULL;
	if (tsk_value_is_valid(&map->hasher_builder)) {
		hasher_builder_type = tsk_value_type(&map->hasher_builder);
		hasher_builder      = tsk_value_data_const(&map->hasher_builder);
	} else {
		hasher_builder_type = tsk_default_hasher_builder_type;
		hasher_builder      = tsk_default_hasher_builder;
	}

	const TskType             *hasher_type = tsk_trait_builder_built_type(hasher_builder_type);
	alignas(max_align_t) TskU8 hasher[tsk_trait_complete_size(hasher_type)];
	tsk_trait_builder_build(
	    hasher_builder_type,
	    hasher_builder,
	    hasher
	);

	tsk_trait_hashable_hash(tsk_map_key_type(map_type), key, hasher_type, &hasher);
	TskU64 hash = tsk_trait_hasher_finalize(hasher_type, hasher);

	tsk_trait_droppable_drop(hasher_type, hasher);

	return hash;
}

TskBoolean tsk_map_is_valid(const TskType *map_type, const TskMap *map) {
	assert(tsk_map_type_is_valid(map_type));

	return map != NULL && ((map->entries != TSK_NULL && map->keys != TSK_NULL && map->values != TSK_NULL) || map->capacity == 0) && map->length <= map->capacity;
}
TskMap tsk_map_new(const TskType *map_type) {
	assert(tsk_map_type_is_valid(map_type));

	TskMap map = {
		.hasher_builder = {
		    .type = TSK_NULL,
		},
		.entries  = TSK_NULL,
		.keys     = TSK_NULL,
		.values   = TSK_NULL,
		.length   = 0,
		.capacity = 0,
	};

	assert(tsk_map_is_valid(map_type, &map));

	return map;
}
TskBoolean tsk_map_with_hasher_builder(const TskType *map_type, TskMap *map, const TskType *hasher_builder_type, TskAny *hasher_builder) {
	assert(tsk_map_type_is_valid(map_type));
	assert(map != TSK_NULL);
	assert(tsk_type_is_valid(hasher_builder_type));
	assert(tsk_type_has_trait(hasher_builder_type, TSK_TRAIT_ID_COMPLETE));
	assert(tsk_type_has_trait(hasher_builder_type, TSK_TRAIT_ID_BUILDER));
	assert(tsk_type_has_trait(tsk_trait_builder_built_type(hasher_builder_type), TSK_TRAIT_ID_HASHER));
	assert(hasher_builder != TSK_NULL);

	if (!tsk_value_new(&map->hasher_builder, hasher_builder_type)) {
		return TSK_FALSE;
	}

	memcpy(
	    tsk_value_data(&map->hasher_builder),
	    hasher_builder,
	    tsk_trait_complete_size(hasher_builder_type)
	);
	map->entries  = TSK_NULL;
	map->keys     = TSK_NULL;
	map->values   = TSK_NULL;
	map->length   = 0;
	map->capacity = 0;

	assert(tsk_map_is_valid(map_type, map));

	return TSK_TRUE;
}
TskEmpty tsk_map_drop(const TskType *map_type, TskMap *map) {
	assert(tsk_map_type_is_valid(map_type));
	assert(tsk_map_is_valid(map_type, map));

	tsk_map_clear(map_type, map);

	if (tsk_value_is_valid(&map->hasher_builder)) {
		tsk_value_drop(&map->hasher_builder);
	}

	free(map->entries);
	if (tsk_trait_complete_size(tsk_map_key_type(map_type)) != 0) {
		free(map->keys);
	}
	if (tsk_trait_complete_size(tsk_map_value_type(map_type)) != 0) {
		free(map->values);
	}

	map->entries  = TSK_NULL;
	map->keys     = TSK_NULL;
	map->values   = TSK_NULL;
	map->capacity = 0;
}
TskBoolean tsk_map_clone(const TskType *map_type, const TskMap *map_1, TskMap *map_2) {
	assert(tsk_map_type_is_valid(map_type));
	assert(tsk_map_is_valid(map_type, map_1));
	assert(map_2 != TSK_NULL);

	if (tsk_map_is_empty(map_type, map_1)) {
		*map_2 = tsk_map_new(map_type);
		return TSK_TRUE;
	}

	TskMap map = tsk_map_new(map_type);
	if (!tsk_map_reserve_additional(map_type, &map, tsk_map_length(map_type, map_1))) {
		return TSK_FALSE;
	}

	for (TskUSize i = 0; i < tsk_map_capacity(map_type, map_1); i++) {
		if (map_1->entries[i].state == TSK_MAP_ENTRY_STATE_OCCUPIED) {
			TskUSize index = map_1->entries[i].hash % tsk_map_capacity(map_type, &map);
			while (map.entries[index].state == TSK_MAP_ENTRY_STATE_OCCUPIED) {
				index = (index + 1) % tsk_map_capacity(map_type, &map);
			}

			if (!tsk_trait_clonable_clone(
			        tsk_map_key_type(map_type),
			        tsk_map_get_key_const(map_type, map_1, i),
			        tsk_map_get_key(map_type, &map, index)
			    )) {
				tsk_map_drop(map_type, &map);
				return TSK_FALSE;
			}

			if (!tsk_trait_clonable_clone(
			        tsk_map_value_type(map_type),
			        tsk_map_get_value_const(map_type, map_1, i),
			        tsk_map_get_value(map_type, &map, index)
			    )) {
				tsk_trait_droppable_drop(
				    tsk_map_key_type(map_type),
				    tsk_map_get_key(map_type, &map, index)
				);
				tsk_map_drop(map_type, &map);
				return TSK_FALSE;
			}

			map.entries[index] = map_1->entries[i];
		}
	}

	map.length = map_1->length;

	*map_2     = map;

	return TSK_TRUE;
}
const TskType *tsk_map_key_type(const TskType *map_type) {
	assert(tsk_map_type_is_valid(map_type));

	return ((const TskMapType *)map_type)->key_type;
}
const TskType *tsk_map_value_type(const TskType *map_type) {
	assert(tsk_map_type_is_valid(map_type));

	return ((const TskMapType *)map_type)->value_type;
}
TskUSize tsk_map_length(const TskType *map_type, const TskMap *map) {
	assert(tsk_map_type_is_valid(map_type));
	assert(tsk_map_is_valid(map_type, map));

	return map->length;
}
TskBoolean tsk_map_is_empty(const TskType *map_type, const TskMap *map) {
	assert(tsk_map_type_is_valid(map_type));
	assert(tsk_map_is_valid(map_type, map));

	return tsk_map_length(map_type, map) == 0;
}
TskUSize tsk_map_capacity(const TskType *map_type, const TskMap *map) {
	assert(tsk_map_type_is_valid(map_type));
	assert(tsk_map_is_valid(map_type, map));

	return map->capacity;
}
TskUSize tsk_map_maximum_capacity(const TskType *map_type) {
	assert(tsk_map_type_is_valid(map_type));

	TskUSize entry_size   = sizeof(TskMapEntry);
	TskUSize key_size     = tsk_trait_complete_size(tsk_map_key_type(map_type));
	TskUSize value_size   = tsk_trait_complete_size(tsk_map_value_type(map_type));

	TskUSize maximum_size = entry_size > key_size ? entry_size : key_size;
	maximum_size          = maximum_size > value_size ? maximum_size : value_size;

	return SIZE_MAX / maximum_size;
}
TskF32 tsk_map_load_factor(const TskType *map_type, const TskMap *map) {
	assert(tsk_map_type_is_valid(map_type));
	assert(tsk_map_is_valid(map_type, map));

	if (tsk_map_capacity(map_type, map) == 0) {
		return 0.0F;
	}

	return (TskF32)tsk_map_length(map_type, map) / (TskF32)tsk_map_capacity(map_type, map);
}
TskF32 tsk_map_maximum_load_factor(const TskType *map_type) {
	assert(tsk_map_type_is_valid(map_type));

	return 0.5F;
}
TskAny *tsk_map_get(const TskType *map_type, TskMap *map, const TskAny *key) {
	assert(tsk_map_type_is_valid(map_type));
	assert(tsk_map_is_valid(map_type, map));
	assert(key != TSK_NULL);

	if (tsk_map_is_empty(map_type, map)) {
		return TSK_NULL;
	}

	TskU64   hash           = tsk_map_hash_key(map_type, map, key);
	TskUSize starting_index = hash % tsk_map_capacity(map_type, map);
	TskUSize index          = starting_index;
	while (map->entries[index].state != TSK_MAP_ENTRY_STATE_EMPTY) {
		if (map->entries[index].state == TSK_MAP_ENTRY_STATE_OCCUPIED &&
		    map->entries[index].hash == hash &&
		    tsk_trait_equatable_equals(
		        tsk_map_key_type(map_type),
		        tsk_map_get_key_const(map_type, map, index),
		        key
		    )) {
			return tsk_map_get_value(map_type, map, index);
		}
		index = (index + 1) % tsk_map_capacity(map_type, map);
		if (index == starting_index) {
			break;
		}
	}

	return TSK_NULL;
}
const TskAny *tsk_map_get_const(const TskType *map_type, const TskMap *map, const TskAny *key) {
	assert(tsk_map_type_is_valid(map_type));
	assert(tsk_map_is_valid(map_type, map));
	assert(key != TSK_NULL);

	if (tsk_map_is_empty(map_type, map)) {
		return TSK_NULL;
	}

	TskU64   hash           = tsk_map_hash_key(map_type, map, key);
	TskUSize starting_index = hash % tsk_map_capacity(map_type, map);
	TskUSize index          = starting_index;
	while (map->entries[index].state != TSK_MAP_ENTRY_STATE_EMPTY) {
		if (map->entries[index].state == TSK_MAP_ENTRY_STATE_OCCUPIED &&
		    map->entries[index].hash == hash &&
		    tsk_trait_equatable_equals(
		        tsk_map_key_type(map_type),
		        tsk_map_get_key_const(map_type, map, index),
		        key
		    )) {
			return tsk_map_get_value_const(map_type, map, index);
		}
		index = (index + 1) % tsk_map_capacity(map_type, map);
		if (index == starting_index) {
			break;
		}
	}

	return TSK_NULL;
}
TskEmpty tsk_map_clear(const TskType *map_type, TskMap *map) {
	assert(tsk_map_type_is_valid(map_type));
	assert(tsk_map_is_valid(map_type, map));
	assert(tsk_type_has_trait(tsk_map_key_type(map_type), TSK_TRAIT_ID_DROPPABLE));
	assert(tsk_type_has_trait(tsk_map_value_type(map_type), TSK_TRAIT_ID_DROPPABLE));

	for (TskUSize i = 0; i < tsk_map_capacity(map_type, map); i++) {
		if (map->entries[i].state == TSK_MAP_ENTRY_STATE_OCCUPIED) {
			tsk_trait_droppable_drop(
			    tsk_map_key_type(map_type),
			    tsk_map_get_key(map_type, map, i)
			);
			tsk_trait_droppable_drop(
			    tsk_map_value_type(map_type),
			    tsk_map_get_value(map_type, map, i)
			);
		}
	}

	map->length = 0;
}
TskBoolean tsk_map_reserve(const TskType *map_type, TskMap *map, TskUSize capacity) {
	assert(tsk_map_type_is_valid(map_type));
	assert(tsk_map_is_valid(map_type, map));

	if (capacity <= tsk_map_capacity(map_type, map)) {
		return TSK_TRUE;
	}

	if (capacity > tsk_map_maximum_capacity(map_type)) {
		return TSK_FALSE;
	}

	TskMapEntry *entries = calloc(capacity, sizeof(TskMapEntry));
	if (entries == TSK_NULL) {
		return TSK_FALSE;
	}

	TskAny *keys = TSK_NULL;
	if (tsk_trait_complete_size(tsk_map_key_type(map_type)) == 0) {
		keys = (TskAny *)tsk_trait_complete_alignment(tsk_map_key_type(map_type)); // NOLINT(performance-no-int-to-ptr)
	} else {
		keys = malloc(capacity * tsk_trait_complete_size(tsk_map_key_type(map_type)));
		if (keys == TSK_NULL) {
			free(entries);
			return TSK_FALSE;
		}
	}

	TskAny *values = TSK_NULL;
	if (tsk_trait_complete_size(tsk_map_value_type(map_type)) == 0) {
		values = (TskAny *)tsk_trait_complete_alignment(tsk_map_value_type(map_type)); // NOLINT(performance-no-int-to-ptr)
	} else {
		values = malloc(capacity * tsk_trait_complete_size(tsk_map_value_type(map_type)));
		if (values == TSK_NULL) {
			free(entries);
			if (tsk_trait_complete_size(tsk_map_key_type(map_type)) != 0) {
				free(keys);
			}
			return TSK_FALSE;
		}
	}

	for (TskUSize i = 0; i < tsk_map_capacity(map_type, map); i++) {
		if (map->entries[i].state == TSK_MAP_ENTRY_STATE_OCCUPIED) {
			TskUSize index = map->entries[i].hash % capacity;
			while (entries[index].state == TSK_MAP_ENTRY_STATE_OCCUPIED) {
				index = (index + 1) % capacity;
			}

			entries[index] = map->entries[i];
			memcpy(
			    (TskU8 *)keys + (index * tsk_trait_complete_size(tsk_map_key_type(map_type))),
			    tsk_map_get_key_const(map_type, map, i),
			    tsk_trait_complete_size(tsk_map_key_type(map_type))
			);
			memcpy(
			    (TskU8 *)values + (index * tsk_trait_complete_size(tsk_map_value_type(map_type))),
			    tsk_map_get_value_const(map_type, map, i),
			    tsk_trait_complete_size(tsk_map_value_type(map_type))
			);
		}
	}

	free(map->entries);
	if (tsk_trait_complete_size(tsk_map_key_type(map_type)) != 0) {
		free(map->keys);
	}
	if (tsk_trait_complete_size(tsk_map_value_type(map_type)) != 0) {
		free(map->values);
	}

	map->entries  = entries;
	map->keys     = keys;
	map->values   = values;
	map->capacity = capacity;

	return TSK_TRUE;
}
TskBoolean tsk_map_reserve_additional(const TskType *map_type, TskMap *map, TskUSize additional) {
	assert(tsk_map_type_is_valid(map_type));
	assert(tsk_map_is_valid(map_type, map));

	TskUSize maximum_capacity = tsk_map_maximum_capacity(map_type);
	if (additional > maximum_capacity - tsk_map_length(map_type, map)) {
		return TSK_FALSE;
	}

	if ((TskF32)tsk_map_capacity(map_type, map) >= (TskF32)(tsk_map_length(map_type, map) + additional) / tsk_map_maximum_load_factor(map_type)) {
		assert(tsk_map_capacity(map_type, map) >= tsk_map_length(map_type, map) + additional);
		return TSK_TRUE;
	}

	TskUSize capacity = tsk_map_capacity(map_type, map);
	do {
		if (capacity == 0) {
			capacity = 1;
		} else if (capacity > maximum_capacity / 2) {
			capacity = maximum_capacity;
			break;
		} else {
			capacity *= 2;
		}
	} while ((TskF32)capacity < (TskF32)(tsk_map_length(map_type, map) + additional) / tsk_map_maximum_load_factor(map_type));
	assert(capacity >= tsk_map_length(map_type, map) + additional);

	return tsk_map_reserve(map_type, map, capacity);
}
TskBoolean tsk_map_insert(const TskType *map_type, TskMap *map, TskAny *key, TskAny *value) {
	assert(tsk_map_type_is_valid(map_type));
	assert(tsk_map_is_valid(map_type, map));
	assert(key != TSK_NULL);
	assert(value != TSK_NULL);

	if (!tsk_map_reserve_additional(map_type, map, 1)) {
		return TSK_FALSE;
	}

	TskU64   hash           = tsk_map_hash_key(map_type, map, key);
	TskUSize starting_index = hash % tsk_map_capacity(map_type, map);
	TskUSize index          = starting_index;
	while (map->entries[index].state == TSK_MAP_ENTRY_STATE_OCCUPIED) {
		if (map->entries[index].hash == hash &&
		    tsk_trait_equatable_equals(
		        tsk_map_key_type(map_type),
		        tsk_map_get_key_const(map_type, map, index),
		        key
		    )) {
			assert(tsk_type_has_trait(tsk_map_key_type(map_type), TSK_TRAIT_ID_DROPPABLE));
			assert(tsk_type_has_trait(tsk_map_value_type(map_type), TSK_TRAIT_ID_DROPPABLE));

			tsk_trait_droppable_drop(
			    tsk_map_key_type(map_type),
			    key
			);
			tsk_trait_droppable_drop(
			    tsk_map_value_type(map_type),
			    tsk_map_get_value(map_type, map, index)
			);
			memcpy(
			    tsk_map_get_value(map_type, map, index),
			    value,
			    tsk_trait_complete_size(tsk_map_value_type(map_type))
			);

			return TSK_TRUE;
		}
		index = (index + 1) % tsk_map_capacity(map_type, map);
		if (index == starting_index) {
			return TSK_FALSE;
		}
	}

	map->entries[index].hash  = hash;
	map->entries[index].state = TSK_MAP_ENTRY_STATE_OCCUPIED;

	memcpy(
	    tsk_map_get_key(map_type, map, index),
	    key,
	    tsk_trait_complete_size(tsk_map_key_type(map_type))
	);
	memcpy(
	    tsk_map_get_value(map_type, map, index),
	    value,
	    tsk_trait_complete_size(tsk_map_value_type(map_type))
	);

	map->length++;

	return TSK_TRUE;
}
TskAny *tsk_map_get_or_insert(const TskType *map_type, TskMap *map, TskAny *key, TskAny *value) {
	assert(tsk_map_type_is_valid(map_type));
	assert(tsk_map_is_valid(map_type, map));
	assert(key != TSK_NULL);
	assert(value != TSK_NULL);

	if (!tsk_map_reserve_additional(map_type, map, 1)) {
		return TSK_NULL;
	}

	TskU64   hash           = tsk_map_hash_key(map_type, map, key);
	TskUSize starting_index = hash % tsk_map_capacity(map_type, map);
	TskUSize index          = starting_index;
	while (map->entries[index].state == TSK_MAP_ENTRY_STATE_OCCUPIED) {
		if (map->entries[index].hash == hash &&
		    tsk_trait_equatable_equals(
		        tsk_map_key_type(map_type),
		        tsk_map_get_key_const(map_type, map, index),
		        key
		    )) {
			assert(tsk_type_has_trait(tsk_map_key_type(map_type), TSK_TRAIT_ID_DROPPABLE));
			assert(tsk_type_has_trait(tsk_map_value_type(map_type), TSK_TRAIT_ID_DROPPABLE));

			tsk_trait_droppable_drop(
			    tsk_map_key_type(map_type),
			    key
			);
			tsk_trait_droppable_drop(
			    tsk_map_value_type(map_type),
			    value
			);

			return tsk_map_get_value(map_type, map, index);
		}
		index = (index + 1) % tsk_map_capacity(map_type, map);
		if (index == starting_index) {
			return TSK_FALSE;
		}
	}

	map->entries[index].hash  = hash;
	map->entries[index].state = TSK_MAP_ENTRY_STATE_OCCUPIED;

	memcpy(
	    tsk_map_get_key(map_type, map, index),
	    key,
	    tsk_trait_complete_size(tsk_map_key_type(map_type))
	);
	memcpy(
	    tsk_map_get_value(map_type, map, index),
	    value,
	    tsk_trait_complete_size(tsk_map_value_type(map_type))
	);

	map->length++;

	return tsk_map_get_value(map_type, map, index);
}
TskBoolean tsk_map_remove(const TskType *map_type, TskMap *map, const TskAny *key, TskAny *value) {
	assert(tsk_map_type_is_valid(map_type));
	assert(tsk_map_is_valid(map_type, map));
	assert(key != TSK_NULL);
	assert(tsk_type_has_trait(tsk_map_key_type(map_type), TSK_TRAIT_ID_DROPPABLE));

	if (tsk_map_is_empty(map_type, map)) {
		return TSK_FALSE;
	}

	TskU64   hash           = tsk_map_hash_key(map_type, map, key);
	TskUSize starting_index = hash % tsk_map_capacity(map_type, map);
	TskUSize index          = starting_index;
	while (map->entries[index].state != TSK_MAP_ENTRY_STATE_EMPTY) {
		if (map->entries[index].state == TSK_MAP_ENTRY_STATE_OCCUPIED &&
		    map->entries[index].hash == hash &&
		    tsk_trait_equatable_equals(
		        tsk_map_key_type(map_type),
		        tsk_map_get_key_const(map_type, map, index),
		        key
		    )) {
			map->entries[index].state = TSK_MAP_ENTRY_STATE_DELETED;

			tsk_trait_droppable_drop(
			    tsk_map_key_type(map_type),
			    tsk_map_get_key(map_type, map, index)
			);

			if (value != TSK_NULL) {
				memcpy(
				    value,
				    tsk_map_get_value_const(map_type, map, index),
				    tsk_trait_complete_size(tsk_map_value_type(map_type))
				);
			} else {
				assert(tsk_type_has_trait(tsk_map_value_type(map_type), TSK_TRAIT_ID_DROPPABLE));
				tsk_trait_droppable_drop(
				    tsk_map_value_type(map_type),
				    tsk_map_get_value(map_type, map, index)
				);
			}

			map->length--;

			return TSK_TRUE;
		}
		index = (index + 1) % tsk_map_capacity(map_type, map);
		if (index == starting_index) {
			break;
		}
	}

	return TSK_FALSE;
}
TskBoolean tsk_map_equals(const TskType *map_type, const TskMap *map_1, const TskMap *map_2) {
	assert(tsk_map_type_is_valid(map_type));
	assert(tsk_map_is_valid(map_type, map_1));
	assert(tsk_map_is_valid(map_type, map_2));
	assert(tsk_type_has_trait(tsk_map_value_type(map_type), TSK_TRAIT_ID_EQUATABLE));

	if (tsk_map_length(map_type, map_1) != tsk_map_length(map_type, map_2)) {
		return TSK_FALSE;
	}

	for (TskUSize i = 0; i < tsk_map_capacity(map_type, map_1); i++) {
		if (map_1->entries[i].state == TSK_MAP_ENTRY_STATE_OCCUPIED) {
			const TskAny *key_1   = tsk_map_get_key_const(map_type, map_1, i);
			const TskAny *value_1 = tsk_map_get_value_const(map_type, map_1, i);

			const TskAny *value_2 = tsk_map_get_const(map_type, map_2, key_1);
			if (value_2 == TSK_NULL) {
				return TSK_FALSE;
			}

			if (!tsk_trait_equatable_equals(
			        tsk_map_value_type(map_type),
			        value_1,
			        value_2
			    )) {
				return TSK_FALSE;
			}
		}
	}

	return TSK_TRUE;
}

TskEmpty tsk_map_type_trait_droppable_drop(const TskType *droppable_type, TskAny *droppable) {
	tsk_map_drop(droppable_type, droppable);
}
TskBoolean tsk_map_type_trait_clonable_clone(const TskType *clonable_type, const TskAny *clonable_1, TskAny *clonable_2) {
	return tsk_map_clone(clonable_type, clonable_1, clonable_2);
}
TskBoolean tsk_map_type_trait_equatable_equals(const TskType *equatable_type, const TskAny *equatable_1, const TskAny *equatable_2) {
	return tsk_map_equals(equatable_type, equatable_1, equatable_2);
}
const TskType *tsk_map_type_trait_iterable_iterator_type(const TskType *iterable_type) {
	return tsk_map_iterator_type(tsk_map_key_type(iterable_type), tsk_map_value_type(iterable_type));
}
TskEmpty tsk_map_type_trait_iterable_iterator(const TskType *iterable_type, TskAny *iterable, TskAny *iterator) {
	*(TskMapIterator *)iterator = tsk_map_iterator(iterable_type, iterable);
}
const TskType *tsk_map_type_trait_iterable_const_iterator_type(const TskType *iterable_type) {
	return tsk_map_iterator_const_type(tsk_map_key_type(iterable_type), tsk_map_value_type(iterable_type));
}
TskEmpty tsk_map_type_trait_iterable_const_iterator(const TskType *iterable_type, const TskAny *iterable, TskAny *iterator) {
	*(TskMapIteratorConst *)iterator = tsk_map_iterator_const(iterable_type, iterable);
}

const TskTraitComplete tsk_map_type_trait_complete = {
	.size      = sizeof(TskMap),
	.alignment = alignof(TskMap),
};
const TskTraitDroppable tsk_map_type_trait_droppable = {
	.drop = tsk_map_type_trait_droppable_drop,
};
const TskTraitClonable tsk_map_type_trait_clonable = {
	.clone = tsk_map_type_trait_clonable_clone,
};
const TskTraitEquatable tsk_map_type_trait_equatable = {
	.equals = tsk_map_type_trait_equatable_equals,
};
const TskTraitIterable tsk_map_type_trait_iterable = {
	.iterator_type = tsk_map_type_trait_iterable_iterator_type,
	.iterator      = tsk_map_type_trait_iterable_iterator,
};
const TskTraitIterableConst tsk_map_type_trait_iterable_const = {
	.iterator_type = tsk_map_type_trait_iterable_const_iterator_type,
	.iterator      = tsk_map_type_trait_iterable_const_iterator,
};

#define TSK_MAP_TYPES_CAPACITY ((TskUSize)1 << 7)

TskMapType tsk_map_types[TSK_MAP_TYPES_CAPACITY];

TskBoolean tsk_map_type_is_valid(const TskType *map_type) {
	return tsk_type_is_valid(map_type) &&
	       &tsk_map_types[0] <= (const TskMapType *)map_type && (const TskMapType *)map_type < &tsk_map_types[TSK_MAP_TYPES_CAPACITY];
}
const TskType *tsk_map_type(const TskType *key_type, const TskType *value_type) {
	assert(tsk_type_is_valid(key_type));
	assert(tsk_type_has_trait(key_type, TSK_TRAIT_ID_COMPLETE));
	assert(tsk_type_has_trait(key_type, TSK_TRAIT_ID_EQUATABLE));
	assert(tsk_type_has_trait(key_type, TSK_TRAIT_ID_HASHABLE));
	assert(tsk_type_is_valid(value_type));
	assert(tsk_type_has_trait(value_type, TSK_TRAIT_ID_COMPLETE));

	const TskType             *hasher_type = tsk_trait_builder_built_type(tsk_default_hasher_builder_type);
	alignas(max_align_t) TskU8 hasher[tsk_trait_complete_size(hasher_type)];
	tsk_trait_builder_build(tsk_default_hasher_builder_type, tsk_default_hasher_builder, hasher);

	tsk_trait_hasher_combine(hasher_type, hasher, (const TskU8 *)&key_type, sizeof(key_type));     // NOLINT(bugprone-sizeof-expression)
	tsk_trait_hasher_combine(hasher_type, hasher, (const TskU8 *)&value_type, sizeof(value_type)); // NOLINT(bugprone-sizeof-expression)
	TskU64 hash = tsk_trait_hasher_finalize(hasher_type, hasher);

	tsk_trait_droppable_drop(hasher_type, hasher);

	TskUSize starting_index = hash & (TSK_MAP_TYPES_CAPACITY - 1);
	TskUSize index          = starting_index;
	while (tsk_map_types[index].key_type != TSK_NULL) {
		if (tsk_map_types[index].key_type == key_type && tsk_map_types[index].value_type == value_type) {
			return &tsk_map_types[index].map_type;
		}
		index = (index + 1) & (TSK_MAP_TYPES_CAPACITY - 1);
		if (index == starting_index) {
			return TSK_NULL;
		}
	}

	tsk_map_types[index].map_type.trait_table                                                                                           = &tsk_map_types[index].map_type_trait_table;
	tsk_map_types[index].map_type_trait_table.entries                                                                                   = tsk_map_types[index].map_type_trait_table_entries;
	tsk_map_types[index].map_type_trait_table.capacity                                                                                  = sizeof(tsk_map_types[index].map_type_trait_table_entries) / sizeof(tsk_map_types[index].map_type_trait_table_entries[0]);

	tsk_map_types[index].map_type_trait_table.entries[TSK_TRAIT_ID_COMPLETE & (tsk_map_types[index].map_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
		.trait_id   = TSK_TRAIT_ID_COMPLETE,
		.trait_data = &tsk_map_type_trait_complete,
	};
	if (tsk_type_has_trait(key_type, TSK_TRAIT_ID_DROPPABLE) && tsk_type_has_trait(value_type, TSK_TRAIT_ID_DROPPABLE)) {
		tsk_map_types[index].map_type_trait_table.entries[TSK_TRAIT_ID_DROPPABLE & (tsk_map_types[index].map_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_DROPPABLE,
			.trait_data = &tsk_map_type_trait_droppable,
		};
	}
	if (tsk_type_has_trait(key_type, TSK_TRAIT_ID_CLONABLE) && tsk_type_has_trait(value_type, TSK_TRAIT_ID_CLONABLE)) {
		tsk_map_types[index].map_type_trait_table.entries[TSK_TRAIT_ID_CLONABLE & (tsk_map_types[index].map_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_CLONABLE,
			.trait_data = &tsk_map_type_trait_clonable,
		};
	}
	if (tsk_type_has_trait(key_type, TSK_TRAIT_ID_EQUATABLE) && tsk_type_has_trait(value_type, TSK_TRAIT_ID_EQUATABLE)) {
		tsk_map_types[index].map_type_trait_table.entries[TSK_TRAIT_ID_EQUATABLE & (tsk_map_types[index].map_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
			.trait_id   = TSK_TRAIT_ID_EQUATABLE,
			.trait_data = &tsk_map_type_trait_equatable,
		};
	}
	tsk_map_types[index].map_type_trait_table.entries[TSK_TRAIT_ID_ITERABLE & (tsk_map_types[index].map_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
		.trait_id   = TSK_TRAIT_ID_ITERABLE,
		.trait_data = &tsk_map_type_trait_iterable,
	};
	tsk_map_types[index].map_type_trait_table.entries[TSK_TRAIT_ID_ITERABLE_CONST & (tsk_map_types[index].map_type_trait_table.capacity - 1)] = (TskTypeTraitTableEntry){
		.trait_id   = TSK_TRAIT_ID_ITERABLE_CONST,
		.trait_data = &tsk_map_type_trait_iterable_const,
	};

	tsk_map_types[index].key_type   = key_type;
	tsk_map_types[index].value_type = value_type;

	(void)snprintf(
	    tsk_map_types[index].map_type_name,
	    sizeof(tsk_map_types[index].map_type_name),
	    "TskMap<%s, %s>",
	    tsk_type_name(key_type),
	    tsk_type_name(value_type)
	);
	tsk_map_types[index].map_type.name = tsk_map_types[index].map_type_name;

	const TskType *map_type            = &tsk_map_types[index].map_type;

	assert(tsk_map_type_is_valid(map_type));

	return map_type;
}

typedef struct TskMapIteratorType TskMapIteratorType;
struct TskMapIteratorType {
	TskType        map_iterator_type;
	TskCharacter   map_iterator_type_name[40];
	const TskType *key_type;
	const TskType *value_type;
};

TskBoolean tsk_map_iterator_is_valid(const TskType *map_iterator_type, const TskMapIterator *map_iterator) {
	assert(tsk_map_iterator_type_is_valid(map_iterator_type));

	return map_iterator != TSK_NULL && tsk_map_is_valid(tsk_map_type(tsk_map_iterator_key_type(map_iterator_type), tsk_map_iterator_value_type(map_iterator_type)), map_iterator->map);
}
const TskType *tsk_map_iterator_key_type(const TskType *map_iterator_type) {
	assert(tsk_map_iterator_type_is_valid(map_iterator_type));

	return ((const TskMapIteratorType *)map_iterator_type)->key_type;
}
const TskType *tsk_map_iterator_value_type(const TskType *map_iterator_type) {
	assert(tsk_map_iterator_type_is_valid(map_iterator_type));

	return ((const TskMapIteratorType *)map_iterator_type)->value_type;
}
const TskType *tsk_map_iterator_item_type(const TskType *map_iterator_type) {
	assert(tsk_map_iterator_type_is_valid(map_iterator_type));

	return tsk_tuple_type(
	    (const TskType *[]){
	        tsk_reference_const_type(tsk_map_iterator_key_type(map_iterator_type)),
	        tsk_reference_type(tsk_map_iterator_value_type(map_iterator_type)),
	    },
	    2
	);
}
TskBoolean tsk_map_iterator_next(const TskType *map_iterator_type, TskMapIterator *map_iterator, TskTuple *item) {
	assert(tsk_map_iterator_type_is_valid(map_iterator_type));
	assert(tsk_map_iterator_is_valid(map_iterator_type, map_iterator));

	const TskType *map_type  = tsk_map_type(tsk_map_iterator_key_type(map_iterator_type), tsk_map_iterator_value_type(map_iterator_type));
	const TskType *item_type = tsk_map_iterator_item_type(map_iterator_type);

	while (map_iterator->index < tsk_map_capacity(map_type, map_iterator->map)) {
		if (map_iterator->map->entries[map_iterator->index].state == TSK_MAP_ENTRY_STATE_OCCUPIED) {
			const TskAny *key   = tsk_map_get_key_const(map_type, map_iterator->map, map_iterator->index);
			TskAny       *value = tsk_map_get_value(map_type, map_iterator->map, map_iterator->index);

			memcpy(
			    tsk_tuple_get(item_type, item, 0),
			    &key,
			    tsk_trait_complete_size(tsk_tuple_element_type(item_type, 0))
			);
			memcpy(
			    tsk_tuple_get(item_type, item, 1),
			    &value,
			    tsk_trait_complete_size(tsk_tuple_element_type(item_type, 1))
			);

			map_iterator->index++;

			return TSK_TRUE;
		}
		map_iterator->index++;
	}

	return TSK_FALSE;
}

const TskType *tsk_map_iterator_type_trait_iterator_item_type(const TskType *iterator_type) {
	return tsk_map_iterator_item_type(iterator_type);
}
TskBoolean tsk_map_iterator_type_trait_iterator_next(const TskType *iterator_type, TskAny *iterator, TskAny *item) {
	return tsk_map_iterator_next(iterator_type, iterator, item);
}

// clang-format off
TSK_TYPE(tsk_map_iterator_type_, TskMapIterator,
	TSK_TYPE_TRAIT(tsk_map_iterator_type_, TSK_TRAIT_ID_COMPLETE, &(TskTraitComplete){
		.size      = sizeof(TskMapIterator),
		.alignment = alignof(TskMapIterator),
	}),
	TSK_TYPE_TRAIT(tsk_map_iterator_type_, TSK_TRAIT_ID_DROPPABLE, &(TskTraitDroppable){
		.drop = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_map_iterator_type_, TSK_TRAIT_ID_CLONABLE, &(TskTraitClonable){
		.clone = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_map_iterator_type_, TSK_TRAIT_ID_EQUATABLE, &(TskTraitEquatable){
		.equals = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_map_iterator_type_, TSK_TRAIT_ID_ITERATOR, &(TskTraitIterator){
		.item_type = tsk_map_iterator_type_trait_iterator_item_type,
		.next      = tsk_map_iterator_type_trait_iterator_next,
	}),
);
// clang-format on

#define TSK_MAP_ITERATOR_TYPES_CAPACITY ((TskUSize)1 << 7)

TskMapIteratorType tsk_map_iterator_types[TSK_MAP_ITERATOR_TYPES_CAPACITY];

TskBoolean tsk_map_iterator_type_is_valid(const TskType *map_iterator_type) {
	return tsk_type_is_valid(map_iterator_type) &&
	       &tsk_map_iterator_types[0] <= (const TskMapIteratorType *)map_iterator_type && (const TskMapIteratorType *)map_iterator_type < &tsk_map_iterator_types[TSK_MAP_ITERATOR_TYPES_CAPACITY];
}
const TskType *tsk_map_iterator_type(const TskType *key_type, const TskType *value_type) {
	assert(tsk_type_is_valid(key_type));
	assert(tsk_type_has_trait(key_type, TSK_TRAIT_ID_COMPLETE));
	assert(tsk_type_has_trait(key_type, TSK_TRAIT_ID_EQUATABLE));
	assert(tsk_type_has_trait(key_type, TSK_TRAIT_ID_HASHABLE));
	assert(tsk_type_is_valid(value_type));
	assert(tsk_type_has_trait(value_type, TSK_TRAIT_ID_COMPLETE));

	const TskType             *hasher_type = tsk_trait_builder_built_type(tsk_default_hasher_builder_type);
	alignas(max_align_t) TskU8 hasher[tsk_trait_complete_size(hasher_type)];
	tsk_trait_builder_build(tsk_default_hasher_builder_type, tsk_default_hasher_builder, hasher);

	tsk_trait_hasher_combine(hasher_type, hasher, (const TskU8 *)&key_type, sizeof(key_type));     // NOLINT(bugprone-sizeof-expression)
	tsk_trait_hasher_combine(hasher_type, hasher, (const TskU8 *)&value_type, sizeof(value_type)); // NOLINT(bugprone-sizeof-expression)
	TskU64 hash = tsk_trait_hasher_finalize(hasher_type, hasher);

	tsk_trait_droppable_drop(hasher_type, hasher);

	TskUSize starting_index = hash & (TSK_MAP_ITERATOR_TYPES_CAPACITY - 1);
	TskUSize index          = starting_index;
	while (tsk_map_iterator_types[index].key_type != TSK_NULL) {
		if (tsk_map_iterator_types[index].key_type == key_type && tsk_map_iterator_types[index].value_type == value_type) {
			return &tsk_map_iterator_types[index].map_iterator_type;
		}
		index = (index + 1) & (TSK_MAP_ITERATOR_TYPES_CAPACITY - 1);
		if (index == starting_index) {
			return TSK_NULL;
		}
	}

	tsk_map_iterator_types[index].map_iterator_type = *tsk_map_iterator_type_;
	tsk_map_iterator_types[index].key_type          = key_type;
	tsk_map_iterator_types[index].value_type        = value_type;

	(void)snprintf(
	    tsk_map_iterator_types[index].map_iterator_type_name,
	    sizeof(tsk_map_iterator_types[index].map_iterator_type_name),
	    "TskMapIterator<%s, %s>",
	    tsk_type_name(key_type),
	    tsk_type_name(value_type)
	);
	tsk_map_iterator_types[index].map_iterator_type.name = tsk_map_iterator_types[index].map_iterator_type_name;

	const TskType *map_iterator_type                     = &tsk_map_iterator_types[index].map_iterator_type;

	assert(tsk_map_iterator_type_is_valid(map_iterator_type));

	return map_iterator_type;
}

TskMapIterator tsk_map_iterator(const TskType *map_type, TskMap *map) {
	assert(tsk_map_type_is_valid(map_type));
	assert(tsk_map_is_valid(map_type, map));

	TskMapIterator map_iterator = {
		.map   = map,
		.index = 0,
	};

	assert(tsk_map_iterator_is_valid(tsk_map_iterator_type(tsk_map_key_type(map_type), tsk_map_value_type(map_type)), &map_iterator));

	return map_iterator;
}

typedef struct TskMapIteratorConstType TskMapIteratorConstType;
struct TskMapIteratorConstType {
	TskType        map_iterator_const_type;
	TskCharacter   map_iterator_const_type_name[40];
	const TskType *key_type;
	const TskType *value_type;
};

TskBoolean tsk_map_iterator_const_is_valid(const TskType *map_iterator_type, const TskMapIteratorConst *map_iterator) {
	assert(tsk_map_iterator_const_type_is_valid(map_iterator_type));

	return map_iterator != TSK_NULL && tsk_map_is_valid(tsk_map_type(tsk_map_iterator_const_key_type(map_iterator_type), tsk_map_iterator_const_value_type(map_iterator_type)), map_iterator->map);
}
const TskType *tsk_map_iterator_const_key_type(const TskType *map_iterator_type) {
	assert(tsk_map_iterator_const_type_is_valid(map_iterator_type));

	return ((const TskMapIteratorConstType *)map_iterator_type)->key_type;
}
const TskType *tsk_map_iterator_const_value_type(const TskType *map_iterator_type) {
	assert(tsk_map_iterator_const_type_is_valid(map_iterator_type));

	return ((const TskMapIteratorConstType *)map_iterator_type)->value_type;
}
const TskType *tsk_map_iterator_const_item_type(const TskType *map_iterator_type) {
	assert(tsk_map_iterator_const_type_is_valid(map_iterator_type));

	return tsk_tuple_type(
	    (const TskType *[]){
	        tsk_reference_const_type(tsk_map_iterator_const_key_type(map_iterator_type)),
	        tsk_reference_const_type(tsk_map_iterator_const_value_type(map_iterator_type)),
	    },
	    2
	);
}
TskBoolean tsk_map_iterator_const_next(const TskType *map_iterator_type, TskMapIteratorConst *map_iterator, TskTuple *item) {
	assert(tsk_map_iterator_const_type_is_valid(map_iterator_type));
	assert(tsk_map_iterator_const_is_valid(map_iterator_type, map_iterator));

	const TskType *map_type  = tsk_map_type(tsk_map_iterator_const_key_type(map_iterator_type), tsk_map_iterator_const_value_type(map_iterator_type));
	const TskType *item_type = tsk_map_iterator_const_item_type(map_iterator_type);

	while (map_iterator->index < tsk_map_capacity(map_type, map_iterator->map)) {
		if (map_iterator->map->entries[map_iterator->index].state == TSK_MAP_ENTRY_STATE_OCCUPIED) {
			const TskAny *key   = tsk_map_get_key_const(map_type, map_iterator->map, map_iterator->index);
			const TskAny *value = tsk_map_get_value_const(map_type, map_iterator->map, map_iterator->index);

			memcpy(
			    tsk_tuple_get(item_type, item, 0),
			    &key,
			    tsk_trait_complete_size(tsk_tuple_element_type(item_type, 0))
			);
			memcpy(
			    tsk_tuple_get(item_type, item, 1),
			    &value,
			    tsk_trait_complete_size(tsk_tuple_element_type(item_type, 1))
			);

			map_iterator->index++;

			return TSK_TRUE;
		}
		map_iterator->index++;
	}

	return TSK_FALSE;
}

const TskType *tsk_map_iterator_const_type_trait_iterator_item_type(const TskType *iterator_type) {
	return tsk_map_iterator_const_item_type(iterator_type);
}
TskBoolean tsk_map_iterator_const_type_trait_iterator_next(const TskType *iterator_type, TskAny *iterator, TskAny *item) {
	return tsk_map_iterator_const_next(iterator_type, iterator, item);
}

// clang-format off
TSK_TYPE(tsk_map_iterator_const_type_, TskMapIteratorConst,
	TSK_TYPE_TRAIT(tsk_map_iterator_const_type_, TSK_TRAIT_ID_COMPLETE, &(TskTraitComplete){
		.size      = sizeof(TskMapIteratorConst),
		.alignment = alignof(TskMapIteratorConst),
	}),
	TSK_TYPE_TRAIT(tsk_map_iterator_const_type_, TSK_TRAIT_ID_DROPPABLE, &(TskTraitDroppable){
		.drop = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_map_iterator_const_type_, TSK_TRAIT_ID_CLONABLE, &(TskTraitClonable){
		.clone = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_map_iterator_const_type_, TSK_TRAIT_ID_EQUATABLE, &(TskTraitEquatable){
		.equals = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(tsk_map_iterator_const_type_, TSK_TRAIT_ID_ITERATOR, &(TskTraitIterator){
		.item_type = tsk_map_iterator_const_type_trait_iterator_item_type,
		.next      = tsk_map_iterator_const_type_trait_iterator_next,
	}),
);
// clang-format on

#define TSK_MAP_ITERATOR_CONST_TYPES_CAPACITY ((TskUSize)1 << 7)

TskMapIteratorConstType tsk_map_iterator_const_types[TSK_MAP_ITERATOR_CONST_TYPES_CAPACITY];

TskBoolean tsk_map_iterator_const_type_is_valid(const TskType *map_iterator_type) {
	return tsk_type_is_valid(map_iterator_type) &&
	       &tsk_map_iterator_const_types[0] <= (const TskMapIteratorConstType *)map_iterator_type && (const TskMapIteratorConstType *)map_iterator_type < &tsk_map_iterator_const_types[TSK_MAP_ITERATOR_CONST_TYPES_CAPACITY];
}
const TskType *tsk_map_iterator_const_type(const TskType *key_type, const TskType *value_type) {
	assert(tsk_type_is_valid(key_type));
	assert(tsk_type_has_trait(key_type, TSK_TRAIT_ID_COMPLETE));
	assert(tsk_type_has_trait(key_type, TSK_TRAIT_ID_EQUATABLE));
	assert(tsk_type_has_trait(key_type, TSK_TRAIT_ID_HASHABLE));
	assert(tsk_type_is_valid(value_type));
	assert(tsk_type_has_trait(value_type, TSK_TRAIT_ID_COMPLETE));

	const TskType             *hasher_type = tsk_trait_builder_built_type(tsk_default_hasher_builder_type);
	alignas(max_align_t) TskU8 hasher[tsk_trait_complete_size(hasher_type)];
	tsk_trait_builder_build(tsk_default_hasher_builder_type, tsk_default_hasher_builder, hasher);

	tsk_trait_hasher_combine(hasher_type, hasher, (const TskU8 *)&key_type, sizeof(key_type));     // NOLINT(bugprone-sizeof-expression)
	tsk_trait_hasher_combine(hasher_type, hasher, (const TskU8 *)&value_type, sizeof(value_type)); // NOLINT(bugprone-sizeof-expression)
	TskU64 hash = tsk_trait_hasher_finalize(hasher_type, hasher);

	tsk_trait_droppable_drop(hasher_type, hasher);

	TskUSize starting_index = hash & (TSK_MAP_ITERATOR_CONST_TYPES_CAPACITY - 1);
	TskUSize index          = starting_index;
	while (tsk_map_iterator_const_types[index].key_type != TSK_NULL) {
		if (tsk_map_iterator_const_types[index].key_type == key_type && tsk_map_iterator_const_types[index].value_type == value_type) {
			return &tsk_map_iterator_const_types[index].map_iterator_const_type;
		}
		index = (index + 1) & (TSK_MAP_ITERATOR_CONST_TYPES_CAPACITY - 1);
		if (index == starting_index) {
			return TSK_NULL;
		}
	}

	tsk_map_iterator_const_types[index].map_iterator_const_type = *tsk_map_iterator_const_type_;
	tsk_map_iterator_const_types[index].key_type                = key_type;
	tsk_map_iterator_const_types[index].value_type              = value_type;

	(void)snprintf(
	    tsk_map_iterator_const_types[index].map_iterator_const_type_name,
	    sizeof(tsk_map_iterator_const_types[index].map_iterator_const_type_name),
	    "TskMapIteratorConst<%s, %s>",
	    tsk_type_name(key_type),
	    tsk_type_name(value_type)
	);
	tsk_map_iterator_const_types[index].map_iterator_const_type.name = tsk_map_iterator_const_types[index].map_iterator_const_type_name;

	const TskType *map_iterator_type                                 = &tsk_map_iterator_const_types[index].map_iterator_const_type;

	assert(tsk_map_iterator_const_type_is_valid(map_iterator_type));

	return map_iterator_type;
}

TskMapIteratorConst tsk_map_iterator_const(const TskType *map_type, const TskMap *map) {
	assert(tsk_map_type_is_valid(map_type));
	assert(tsk_map_is_valid(map_type, map));

	TskMapIteratorConst map_iterator = {
		.map   = map,
		.index = 0,
	};

	assert(tsk_map_iterator_const_is_valid(tsk_map_iterator_const_type(tsk_map_key_type(map_type), tsk_map_value_type(map_type)), &map_iterator));

	return map_iterator;
}
