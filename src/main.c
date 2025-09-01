#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <tsk/trait/clonable.h>
#include <tsk/trait/complete.h>
#include <tsk/trait/droppable.h>
#include <tsk/trait/equatable.h>
#include <tsk/trait/hashable.h>
#include <tsk/trait/hasher.h>
#include <tsk/type.h>

#include <tsk/map.h>

TskBoolean string_type_equatable_equals(const TskType *equatable_type, const TskAny *equatable_1, const TskAny *equatable_2) {
	(TskEmpty) equatable_type;

	return strcmp(*(const TskCharacter **)equatable_1, *(const TskCharacter **)equatable_2) == 0;
}
TskEmpty string_type_trait_hashable_hash(const TskType *hashable_type, const TskAny *hashable, const TskType *hasher_type, TskAny *hasher) {
	(TskEmpty) hashable_type;

	TskUSize length = strlen(*(const TskCharacter **)hashable);
	tsk_trait_hasher_combine(
	    hasher_type,
	    hasher,
	    (const TskU8 *)*(const TskCharacter **)hashable,
	    length
	);
}

// clang-format off
TSK_TYPE(string_type, const TskCharacter *,
	TSK_TYPE_TRAIT(string_type, TSK_TRAIT_ID_COMPLETE, &(TskTraitComplete){
		.size      = sizeof(const TskCharacter *),
		.alignment = alignof(const TskCharacter *),
	}),
	TSK_TYPE_TRAIT(string_type, TSK_TRAIT_ID_DROPPABLE, &(TskTraitDroppable){
		.drop = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(string_type, TSK_TRAIT_ID_CLONABLE, &(TskTraitClonable){
		.clone = TSK_NULL,
	}),
	TSK_TYPE_TRAIT(string_type, TSK_TRAIT_ID_EQUATABLE, &(TskTraitEquatable){
		.equals = string_type_equatable_equals,
	}),
	TSK_TYPE_TRAIT(string_type, TSK_TRAIT_ID_HASHABLE, &(TskTraitHashable){
		.hash = string_type_trait_hashable_hash,
	}),
);
// clang-format on

int main(void) {
	const TskType *map_type        = tsk_map_type(string_type, tsk_u32_type);
	TskMap         map             = tsk_map_new(map_type);

	TskCharacter text[]            = "\
The quick brown fox jumps over the lazy dog.\n\
The dog barked, and the fox ran away.\n\
Dogs and foxes are not always enemies; sometimes they share the same forest.\n\
Quick thinking helps a fox escape danger, but lazy habits may cost a dog its meal.\n\
";

	const TskCharacter *delimiters = " \n\t.,;:!?\"'()[]{}<>";
	TskCharacter       *token      = strtok(text, delimiters);
	while (token != TSK_NULL) {
		for (TskCharacter *character = token; *character; character++) {
			*character = (TskCharacter)tolower(*character);
		}

		TskU32 *value = tsk_map_get_or_insert(map_type, &map, &(const TskCharacter *){ token }, &(TskU32){ 0 });
		(*value)++;

		token = strtok(TSK_NULL, delimiters);
	}

	const TskType *map_iterator_type = tsk_map_iterator_type(map_type);
	TskMapIterator map_iterator      = tsk_map_iterator(map_type, &map);

	for (
	    struct { const TskCharacter **key; TskU32 *value; } item;
	    tsk_map_iterator_next(map_iterator_type, &map_iterator, &item);
	) {
		printf("\"%s\": %u\n", *item.key, *item.value);
	}

	tsk_map_drop(map_type, &map);
}
