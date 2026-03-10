#include "tsk/type.h"
#include <tsk/array.h>
#include <tsk/map.h>

#include <assert.h>
#include <stdio.h>

TskBoolean tsk_array_view_next_permutation(const TskType *array_view_type, TskArrayView array_view) {
	assert(tsk_array_view_type_is_valid(array_view_type));
	assert(tsk_array_view_is_valid(array_view_type, array_view));
	assert(tsk_type_has_trait(tsk_array_view_element_type(array_view_type), TSK_TRAIT_ID_COMPARABLE));

	const TskType    *array_view_const_type = tsk_array_view_const_type(tsk_array_view_element_type(array_view_type));
	TskArrayViewConst array_view_const      = tsk_array_view_as_const(array_view_type, array_view);

	array_view                              = tsk_array_view_slice(array_view_type, array_view, tsk_array_view_length(array_view_type, array_view), 0, -1);

	TskUSize i                              = tsk_array_view_const_sorted_until(array_view_const_type, array_view_const);
	if (i == tsk_array_view_length(array_view_type, array_view)) {
		return TSK_FALSE;
	}

	TskUSize j = tsk_array_view_const_upper_bound(
	    array_view_const_type,
	    tsk_array_view_const_slice(array_view_const_type, array_view_const, 0, i, 1),
	    tsk_array_view_get(array_view_type, array_view, i)
	);

	tsk_array_view_swap(array_view_type, array_view, i, j);
	tsk_array_view_reverse(array_view_type, tsk_array_view_slice(array_view_type, array_view, 0, i, 1));

	return TSK_TRUE;
}

TskEmpty tsk_array_view_print(const TskType *array_view_type, TskArrayViewConst array_view, TskEmpty (*print)(const TskAny *element)) {
	assert(tsk_array_view_const_type_is_valid(array_view_type));
	assert(tsk_array_view_const_is_valid(array_view_type, array_view));
	assert(print != TSK_NULL);

	printf("{ ");
	for (TskUSize i = 0; i < tsk_array_view_const_length(array_view_type, array_view); i++) {
		if (i != 0) {
			printf(", ");
		}
		print(tsk_array_view_const_get(array_view_type, array_view, i));
	}
	printf(" }\n");
}

TskEmpty print_i32(const TskAny *element) {
	printf("%d", *(const TskI32 *)element);
}
TskEmpty print_character(const TskAny *element) {
	printf("%c", *(const TskCharacter *)element);
}

TskU64 factorial(TskU64 n) {
	TskU64 result = 1;
	for (TskU64 i = 2; i <= n; i++) {
		result *= i;
	}
	return result;
}

int main(void) {
	TskArray input     = tsk_array_new(tsk_array_type(tsk_character_type));
	TskI32   character = EOF;
	while ((character = getchar()) != EOF && character != '\n') {
		tsk_array_push_back(tsk_array_type(tsk_character_type), &input, &(TskCharacter){ (TskCharacter)character });
	}

	TskMap frequency_map = tsk_map_new(tsk_map_type(tsk_character_type, tsk_u32_type));
	for (TskUSize i = 0; i < tsk_array_length(tsk_array_type(tsk_character_type), &input); i++) {
		TskU32 *count = tsk_map_get_or_insert(tsk_map_type(tsk_character_type, tsk_u32_type), &frequency_map, tsk_array_get(tsk_array_type(tsk_character_type), &input, i), &(TskU32){ 0 });
		(*count)++;
	}

	TskU64 permutation_count = factorial(tsk_array_length(tsk_array_type(tsk_character_type), &input));

	struct {
		TskCharacter key;
		TskI32       value;
	} item;
	for (TskMapIterator iterator = tsk_map_iterator(tsk_map_type(tsk_character_type, tsk_u32_type), &frequency_map); tsk_map_iterator_next(tsk_map_iterator_type(tsk_character_type, tsk_u32_type), &iterator, &item);) {
		printf("'%c': %u\n", item.key, item.value);

		permutation_count /= factorial(item.value);
	}

	printf("Total permutations: %lu\n", permutation_count);

	printf("Permutations:\n");
	TskArrayView input_view = tsk_array_view(tsk_array_type(tsk_character_type), &input);
	tsk_array_view_sort(tsk_array_view_type(tsk_character_type), input_view);

	TskU64 i = 1;
	do {
		printf("%lu. ", i);
		TskArrayViewConst input_view_const = tsk_array_view_as_const(tsk_array_view_type(tsk_character_type), input_view);
		tsk_array_view_print(tsk_array_view_const_type(tsk_character_type), input_view_const, print_character);
		i++;
	} while (tsk_array_view_next_permutation(tsk_array_view_type(tsk_character_type), input_view));

	tsk_map_drop(tsk_map_type(tsk_character_type, tsk_u32_type), &frequency_map);
	tsk_array_drop(tsk_array_type(tsk_character_type), &input);
}
