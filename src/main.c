#include <tsk/array.h>
#include <tsk/trait/complete.h>
#include <tsk/trait/iterator.h>

#include <assert.h>
#include <stdio.h>

TskBoolean is_even(const TskAny *value) {
	return (*(TskU32 *)value % 2) == 0;
}

int main(void) {
	TskArray array = tsk_array_new(tsk_array_type(tsk_u32_type));

	TskUSize size  = 0;
	scanf("%zu", &size); // NOLINT

	for (TskUSize i = 0; i < size; i++) {
		TskU32 value = 0;
		scanf("%u", &value); // NOLINT
		tsk_array_push_back(tsk_array_type(tsk_u32_type), &array, &value);
	}

	tsk_array_view_partition(
	    tsk_array_view_type(tsk_u32_type),
	    tsk_array_view(tsk_array_type(tsk_u32_type), &array),
	    is_even
	);

	TskUSize parition_point =
	    tsk_array_view_const_partition_point(
	        tsk_array_view_const_type(tsk_u32_type),
	        tsk_array_view_const(tsk_array_type(tsk_u32_type), &array),
	        is_even
	    );

	TskArrayView evens = tsk_array_view_slice(
	    tsk_array_view_type(tsk_u32_type),
	    tsk_array_view(tsk_array_type(tsk_u32_type), &array),
	    0,
	    parition_point,
	    1
	);
	tsk_array_view_sort(tsk_array_view_type(tsk_u32_type), evens);

	TskArrayView odds = tsk_array_view_slice(
	    tsk_array_view_type(tsk_u32_type),
	    tsk_array_view(tsk_array_type(tsk_u32_type), &array),
	    parition_point,
	    tsk_array_length(tsk_array_type(tsk_u32_type), &array),
	    1
	);
	tsk_array_view_sort(tsk_array_view_type(tsk_u32_type), odds);

	printf("Evens: ");
	for (TskUSize i = 0; i < tsk_array_view_length(tsk_array_view_type(tsk_u32_type), evens); i++) {
		const TskU32 *value = tsk_array_view_get(tsk_array_view_type(tsk_u32_type), evens, i);
		printf("%u ", *value);
	}
	printf("\n");

	printf("Odds: ");
	for (TskUSize i = 0; i < tsk_array_view_length(tsk_array_view_type(tsk_u32_type), odds); i++) {
		const TskU32 *value = tsk_array_view_get(tsk_array_view_type(tsk_u32_type), odds, i);
		printf("%u ", *value);
	}
	printf("\n");

	tsk_array_drop(tsk_array_type(tsk_u32_type), &array);
}
