#include <tsk/array.h>
#include <tsk/trait/comparable.h>

#include <inttypes.h>
#include <stdio.h>

#include <assert.h>

TskEmpty tsk_heap_heapify(const TskType *array_view_type, TskArrayView array_view, TskUSize i) {
	assert(tsk_array_view_type_is_valid(array_view_type));
	assert(tsk_array_view_is_valid(array_view_type, array_view));
	assert(tsk_type_has_trait(tsk_array_view_element_type(array_view_type), TSK_TRAIT_ID_COMPARABLE));
	assert(i < tsk_array_view_length(array_view_type, array_view));

	TskUSize left     = (2 * i) + 1;
	TskUSize right    = (2 * i) + 2;

	TskUSize smallest = i;
	if (left < tsk_array_view_length(array_view_type, array_view) &&
	    tsk_trait_comparable_compare(
	        tsk_array_view_element_type(array_view_type),
	        tsk_array_view_get(array_view_type, array_view, left),
	        tsk_array_view_get(array_view_type, array_view, smallest)
	    ) < 0) {
		smallest = left;
	}
	if (right < tsk_array_view_length(array_view_type, array_view) &&
	    tsk_trait_comparable_compare(
	        tsk_array_view_element_type(array_view_type),
	        tsk_array_view_get(array_view_type, array_view, right),
	        tsk_array_view_get(array_view_type, array_view, smallest)
	    ) < 0) {
		smallest = right;
	}

	if (smallest != i) {
		tsk_array_view_swap(array_view_type, array_view, i, smallest);
		tsk_heap_heapify(array_view_type, array_view, smallest);
	}
}

TskEmpty tsk_heap_build(const TskType *array_view_type, TskArrayView array_view) {
	for (TskISize i = (TskISize)(tsk_array_view_length(array_view_type, array_view) / 2); i >= 0; i--) {
		tsk_heap_heapify(array_view_type, array_view, i);
	}
}

TskEmpty tsk_heap_sort(const TskType *array_view_type, TskArrayView array_view) {
	assert(tsk_array_view_type_is_valid(array_view_type));
	assert(tsk_array_view_is_valid(array_view_type, array_view));
	assert(tsk_type_has_trait(tsk_array_view_element_type(array_view_type), TSK_TRAIT_ID_COMPARABLE));

	tsk_heap_build(array_view_type, array_view);

	for (TskUSize i = tsk_array_view_length(array_view_type, array_view) - 1; i > 0; i--) {
		tsk_array_view_swap(array_view_type, array_view, 0, i);
		tsk_heap_heapify(array_view_type, tsk_array_view_slice(array_view_type, array_view, 0, i, 1), 0);
	}
}

int main(void) {
	TskU32       array[]    = { 3, 1, 4, 1, 5, 9, 2, 6, 5, 3 };
	TskArrayView array_view = tsk_array_view_new(tsk_array_view_type(tsk_u32_type), array, sizeof(array) / sizeof(*array), 1);

	for (TskUSize i = 0; i < tsk_array_view_length(tsk_array_view_type(tsk_u32_type), array_view); i++) {
		printf("%" PRIu32 " ", *(TskU32 *)tsk_array_view_get(tsk_array_view_type(tsk_u32_type), array_view, i));
	}
	printf("\n");

	tsk_heap_sort(tsk_array_view_type(tsk_u32_type), array_view);

	for (TskUSize i = 0; i < tsk_array_view_length(tsk_array_view_type(tsk_u32_type), array_view); i++) {
		printf("%" PRIu32 " ", *(TskU32 *)tsk_array_view_get(tsk_array_view_type(tsk_u32_type), array_view, i));
	}
	printf("\n");
}
