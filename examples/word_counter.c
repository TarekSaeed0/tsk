#include <assert.h>
#include <stdio.h>

#include <tsk/trait/clonable.h>
#include <tsk/trait/complete.h>
#include <tsk/trait/droppable.h>
#include <tsk/trait/equatable.h>
#include <tsk/trait/hashable.h>
#include <tsk/trait/hasher.h>
#include <tsk/type.h>

#include <tsk/array.h>
#include <tsk/map.h>

TskArrayViewConst tsk_array_view_const_tokenize(const TskType *array_view_const_type, TskArrayViewConst *array_view_const, TskArrayViewConst delimiters) {
	assert(tsk_array_view_const_type_is_valid(array_view_const_type));
	assert(array_view_const != TSK_NULL);
	assert(tsk_array_view_const_is_valid(array_view_const_type, *array_view_const));
	assert(tsk_array_view_const_is_valid(array_view_const_type, delimiters));

	TskUSize i = 0;

	while (i < tsk_array_view_const_length(array_view_const_type, *array_view_const) &&
	       tsk_array_view_const_linear_search(
	           array_view_const_type,
	           delimiters,
	           tsk_array_view_const_get(array_view_const_type, *array_view_const, i),
	           TSK_NULL
	       )) {
		i++;
	}
	TskUSize start = i;

	while (i < tsk_array_view_const_length(array_view_const_type, *array_view_const) &&
	       !tsk_array_view_const_linear_search(
	           array_view_const_type,
	           delimiters,
	           tsk_array_view_const_get(array_view_const_type, *array_view_const, i),
	           TSK_NULL
	       )) {
		i++;
	}
	TskUSize end            = i;

	TskArrayViewConst token = tsk_array_view_const_slice(array_view_const_type, *array_view_const, start, end, 1);

	*array_view_const       = tsk_array_view_const_slice(
      array_view_const_type,
      *array_view_const,
      end,
      tsk_array_view_const_length(array_view_const_type, *array_view_const),
      1
  );

	return token;
}

int main(void) {
	TskArray text    = tsk_array_new(tsk_array_type(tsk_character_type));

	TskI32 character = EOF;
	while ((character = getchar()) != EOF && character != '\n') {
		tsk_array_push_back(
		    tsk_array_type(tsk_character_type),
		    &text,
		    &(TskCharacter){ (TskCharacter)character }
		);
	}

	TskArrayViewConst array_view = tsk_array_view_const(tsk_array_type(tsk_character_type), &text);
	TskArrayViewConst delimiters = tsk_array_view_const_new(
	    tsk_array_view_const_type(tsk_character_type),
	    " \n\t.,;:!?\"'()[]{}<>",
	    19,
	    1
	);

	TskMap words_frequency = tsk_map_new(tsk_map_type(tsk_array_view_const_type(tsk_character_type), tsk_u32_type));

	TskArrayViewConst word;
	while (!tsk_array_view_const_is_empty(
	    tsk_array_view_const_type(tsk_character_type),
	    word = tsk_array_view_const_tokenize(
	        tsk_array_view_const_type(tsk_character_type),
	        &array_view,
	        delimiters
	    )
	)) {
		TskU32 *frequency = tsk_map_get_or_insert(
		    tsk_map_type(tsk_array_view_const_type(tsk_character_type), tsk_u32_type),
		    &words_frequency,
		    &word,
		    &(TskU32){ 0 }
		);
		(*frequency)++;
	}

	TskMapIteratorConst word_frequency_iterator = tsk_map_iterator_const(
	    tsk_map_type(tsk_array_view_const_type(tsk_character_type), tsk_u32_type),
	    &words_frequency
	);

	for (
	    struct { const TskArrayViewConst *key; const TskU32 *value; } item;
	    tsk_map_iterator_const_next(
	        tsk_map_iterator_const_type(tsk_array_view_const_type(tsk_character_type), tsk_u32_type),
	        &word_frequency_iterator,
	        &item
	    );
	) {
		printf(
		    "\"%.*s\": %u\n",
		    (TskI32)tsk_array_view_const_length(tsk_array_view_const_type(tsk_character_type), *item.key),
		    (const TskCharacter *)tsk_array_view_const_elements(tsk_array_view_const_type(tsk_character_type), *item.key),
		    *item.value
		);
	}

	tsk_map_drop(tsk_map_type(tsk_array_view_const_type(tsk_character_type), tsk_u32_type), &words_frequency);
	tsk_array_drop(tsk_array_type(tsk_character_type), &text);
}
