#include <tsk/array.h>
#include <tsk/map.h>

#include <assert.h>
#include <stdio.h>

TskBool tsk_array_view_next_permutation(TskArrayView array_view) {
	assert(tsk_array_view_is_valid(array_view));
	assert(tsk_type_has_trait(array_view.element_type, TSK_TRAIT_COMPARABLE));

	array_view = tsk_array_view_slice(array_view, tsk_array_view_length(array_view), 0, -1);

	TskUSize i = tsk_array_view_sorted_until(array_view);
	if (i == tsk_array_view_length(array_view)) {
		return TSK_FALSE;
	}

	TskUSize j = tsk_array_view_upper_bound(
	    tsk_array_view_slice(array_view, 0, i, 1),
	    tsk_array_view_get(array_view, i)
	);

	tsk_array_view_swap(array_view, i, j);
	tsk_array_view_reverse(tsk_array_view_slice(array_view, 0, i, 1));

	return TSK_TRUE;
}

TskEmpty tsk_array_view_print(TskArrayView array_view, TskEmpty (*print)(const TskAny *element)) {
	assert(tsk_array_view_is_valid(array_view));
	assert(print != TSK_NULL);

	printf("{ ");
	for (TskUSize i = 0; i < tsk_array_view_length(array_view); i++) {
		if (i != 0) {
			printf(", ");
		}
		print(tsk_array_view_get(array_view, i));
	}
	printf(" }\n");
}

TskEmpty print_i32(const TskAny *element) {
	printf("%d", *(const TskI32 *)element);
}
TskEmpty print_char(const TskAny *element) {
	printf("%c", *(const TskChar *)element);
}

TskU64 factorial(TskU64 n) {
	TskU64 result = 1;
	for (TskU64 i = 2; i <= n; i++) {
		result *= i;
	}
	return result;
}

int main(void) {
	TskArray input     = tsk_array_new(&tsk_char_type);
	TskI32   character = EOF;
	while ((character = getchar()) != EOF && character != '\n') {
		tsk_array_push_back(&input, &(TskChar){ (TskChar)character });
	}

	TskMap frequency_map = tsk_map_new(&tsk_char_type, &tsk_u32_type);
	for (TskUSize i = 0; i < tsk_array_length(&input); i++) {
		TskU32 *count = tsk_map_get_or_insert(&frequency_map, tsk_array_get(&input, i), &(TskU32){ 0 });
		(*count)++;
	}

	TskU64 permutation_count = factorial(tsk_array_length(&input));

	TskMapIteratorItem item;
	for (TskMapIterator iterator = tsk_map_iterator(&frequency_map); tsk_map_iterator_next(&iterator, &item);) {
		printf("'%c': %u\n", *(const TskChar *)item.key, *(const TskU32 *)item.value);

		permutation_count /= factorial(*(const TskU32 *)item.value);
	}

	printf("Total permutations: %lu\n", permutation_count);

	printf("Permutations:\n");
	TskArrayView input_view = tsk_array_to_view(&input);
	tsk_array_view_sort(input_view);

	TskU64 i = 1;
	do {
		printf("%lu. ", i);
		tsk_array_view_print(input_view, print_char);
		i++;
	} while (tsk_array_view_next_permutation(input_view));

	tsk_map_drop(&frequency_map);
	tsk_array_drop(&input);
}
