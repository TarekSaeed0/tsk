#include <tsk/array.h>

#include <assert.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

TskBool tsk_array_is_valid(const TskArray *array) {
	return array != TSK_NULL && tsk_type_is_valid(array->element_type) &&
	       tsk_type_has_trait(array->element_type, TSK_TRAIT_COMPLETE) &&
	       (array->elements != TSK_NULL || array->capacity == 0) && array->length <= array->capacity;
}
TskArray tsk_array_new(const TskType *element_type) {
	assert(tsk_type_is_valid(element_type));
	assert(tsk_type_has_trait(element_type, TSK_TRAIT_COMPLETE));

	TskArray array = {
		.element_type = element_type,
		.length       = 0,
	};

	if (tsk_trait_complete_size(tsk_type_trait(element_type, TSK_TRAIT_COMPLETE)) == 0) {
		array.elements = (TskAny *)tsk_trait_complete_alignment(tsk_type_trait(element_type, TSK_TRAIT_COMPLETE)); // NOLINT(performance-no-int-to-ptr)
		array.capacity = SIZE_MAX;
	} else {
		array.elements = TSK_NULL;
		array.capacity = 0;
	}

	assert(tsk_array_is_valid(&array));

	return array;
}
TskEmpty tsk_array_drop(TskArray *array) {
	assert(tsk_array_is_valid(array));

	tsk_array_clear(array);

	if (tsk_trait_complete_size(tsk_type_trait(array->element_type, TSK_TRAIT_COMPLETE)) != 0) {
		free(array->elements);
	}

	array->elements = TSK_NULL;
	array->capacity = 0;
}
TskBool tsk_array_clone(const TskArray *array_1, TskArray *array_2) {
	assert(tsk_array_is_valid(array_1));
	assert(array_2 != TSK_NULL);

	if (tsk_array_is_empty(array_1)) {
		*array_2 = tsk_array_new(array_1->element_type);
		return TSK_TRUE;
	}

	TskAny *elements = TSK_NULL;
	if (tsk_trait_complete_size(tsk_type_trait(array_1->element_type, TSK_TRAIT_COMPLETE)) == 0) {
		elements = (TskAny *)tsk_trait_complete_alignment(tsk_type_trait(array_1->element_type, TSK_TRAIT_COMPLETE)); // NOLINT(performance-no-int-to-ptr)
	} else {
		elements = malloc(tsk_array_length(array_1) * tsk_trait_complete_size(tsk_type_trait(array_1->element_type, TSK_TRAIT_COMPLETE)));
		if (elements == TSK_NULL) {
			return TSK_FALSE;
		}
	}

	for (TskUSize i = 0; i < tsk_array_length(array_1); i++) {
		if (!tsk_trait_clonable_clone(
		        tsk_type_trait(array_1->element_type, TSK_TRAIT_COMPLETE),
		        tsk_array_get_const(array_1, i),
		        (TskU8 *)elements + (i * tsk_trait_complete_size(tsk_type_trait(array_1->element_type, TSK_TRAIT_COMPLETE)))
		    )) {
			for (TskUSize j = 0; j < i; j++) {
				tsk_trait_droppable_drop(
				    tsk_type_trait(array_1->element_type, TSK_TRAIT_DROPPABLE),
				    (TskU8 *)elements + (j * tsk_trait_complete_size(tsk_type_trait(array_1->element_type, TSK_TRAIT_COMPLETE)))
				);
			}
			free(elements);
			return TSK_FALSE;
		}
	}

	array_2->element_type = array_1->element_type;
	array_2->elements     = elements;
	array_2->length       = array_1->length;
	array_2->capacity     = array_1->capacity;

	return TSK_TRUE;
}
TskUSize tsk_array_length(const TskArray *array) {
	assert(tsk_array_is_valid(array));

	return array->length;
}
TskBool tsk_array_is_empty(const TskArray *array) {
	assert(tsk_array_is_valid(array));

	return tsk_array_length(array) == 0;
}
TskUSize tsk_array_capacity(const TskArray *array) {
	assert(tsk_array_is_valid(array));

	return array->capacity;
}
TskUSize tsk_array_maximum_capacity(const TskArray *array) {
	assert(tsk_array_is_valid(array));

	TskUSize element_size = tsk_trait_complete_size(tsk_type_trait(array->element_type, TSK_TRAIT_COMPLETE));
	return element_size == 0 ? SIZE_MAX : SIZE_MAX / element_size;
}
TskAny *tsk_array_get(TskArray *array, TskUSize index) {
	assert(tsk_array_is_valid(array));
	assert(index < tsk_array_length(array));

	return (TskU8 *)array->elements + (index * tsk_trait_complete_size(tsk_type_trait(array->element_type, TSK_TRAIT_COMPLETE)));
}
const TskAny *tsk_array_get_const(const TskArray *array, TskUSize index) {
	assert(tsk_array_is_valid(array));
	assert(index < tsk_array_length(array));

	return (const TskU8 *)array->elements + (index * tsk_trait_complete_size(tsk_type_trait(array->element_type, TSK_TRAIT_COMPLETE)));
}
TskAny *tsk_array_front(TskArray *array) {
	assert(tsk_array_is_valid(array));

	if (tsk_array_is_empty(array)) {
		return TSK_NULL;
	}

	return tsk_array_get(array, 0);
}
const TskAny *tsk_array_front_const(const TskArray *array) {
	assert(tsk_array_is_valid(array));

	if (tsk_array_is_empty(array)) {
		return TSK_NULL;
	}

	return tsk_array_get_const(array, 0);
}
TskAny *tsk_array_back(TskArray *array) {
	assert(tsk_array_is_valid(array));

	if (tsk_array_is_empty(array)) {
		return TSK_NULL;
	}

	return tsk_array_get(array, tsk_array_length(array) - 1);
}
const TskAny *tsk_array_back_const(const TskArray *array) {
	assert(tsk_array_is_valid(array));

	if (tsk_array_is_empty(array)) {
		return TSK_NULL;
	}

	return tsk_array_get_const(array, tsk_array_length(array) - 1);
}
TskEmpty tsk_array_swap(TskArray *array, TskUSize index_1, TskUSize index_2) {
	tsk_array_view_swap(tsk_array_to_view(array), index_1, index_2);
}
TskEmpty tsk_array_clear(TskArray *array) {
	assert(tsk_array_is_valid(array));
	assert(tsk_type_has_trait(array->element_type, TSK_TRAIT_DROPPABLE));

	for (TskUSize i = 0; i < tsk_array_length(array); i++) {
		tsk_trait_droppable_drop(
		    tsk_type_trait(array->element_type, TSK_TRAIT_DROPPABLE),
		    tsk_array_get(array, i)
		);
	}

	array->length = 0;
}
TskBool tsk_array_reserve(TskArray *array, TskUSize capacity) {
	assert(tsk_array_is_valid(array));

	if (capacity <= tsk_array_capacity(array)) {
		return TSK_TRUE;
	}

	if (capacity > tsk_array_maximum_capacity(array)) {
		return TSK_FALSE;
	}

	TskAny *elements = realloc(array->elements, capacity * tsk_trait_complete_size(tsk_type_trait(array->element_type, TSK_TRAIT_COMPLETE)));
	if (elements == TSK_NULL) {
		return TSK_FALSE;
	}

	array->elements = elements;
	array->capacity = capacity;

	return TSK_TRUE;
}
TskBool tsk_array_reserve_additional(TskArray *array, TskUSize additional) {
	assert(tsk_array_is_valid(array));

	TskUSize maximum_capacity = tsk_array_maximum_capacity(array);
	if (additional > maximum_capacity - tsk_array_length(array)) {
		return TSK_FALSE;
	}

	if (tsk_array_capacity(array) >= tsk_array_length(array) + additional) {
		return TSK_TRUE;
	}

	TskUSize capacity = tsk_array_capacity(array);
	do {
		if (capacity == 0) {
			capacity = 1;
		} else if (capacity > maximum_capacity / 2) {
			capacity = maximum_capacity;
			break;
		} else {
			capacity *= 2;
		}
	} while (capacity < tsk_array_length(array) + additional);

	return tsk_array_reserve(array, capacity);
}
TskBool tsk_array_insert(TskArray *array, TskUSize index, TskAny *elements, TskUSize count) {
	assert(tsk_array_is_valid(array));
	assert(index <= tsk_array_length(array));
	assert(elements != TSK_NULL || count == 0);

	if (!tsk_array_reserve_additional(array, count)) {
		return TSK_FALSE;
	}

	memmove(
	    (TskU8 *)array->elements + ((index + count) * tsk_trait_complete_size(tsk_type_trait(array->element_type, TSK_TRAIT_COMPLETE))),
	    (const TskU8 *)array->elements + (index * tsk_trait_complete_size(tsk_type_trait(array->element_type, TSK_TRAIT_COMPLETE))),
	    (tsk_array_length(array) - index) * tsk_trait_complete_size(tsk_type_trait(array->element_type, TSK_TRAIT_COMPLETE))
	);
	memcpy(
	    (TskU8 *)array->elements + (index * tsk_trait_complete_size(tsk_type_trait(array->element_type, TSK_TRAIT_COMPLETE))),
	    elements,
	    count * tsk_trait_complete_size(tsk_type_trait(array->element_type, TSK_TRAIT_COMPLETE))
	);

	array->length += count;

	return TSK_TRUE;
}
TskEmpty tsk_array_remove(TskArray *array, TskUSize index, TskAny *elements, TskUSize count) {
	assert(tsk_array_is_valid(array));
	assert(index < tsk_array_length(array));
	assert(count < tsk_array_length(array) - index);

	if (elements != TSK_NULL) {
		memcpy(
		    elements,
		    tsk_array_get_const(array, index),
		    count * tsk_trait_complete_size(tsk_type_trait(array->element_type, TSK_TRAIT_COMPLETE))
		);
	} else {
		assert(tsk_type_has_trait(array->element_type, TSK_TRAIT_DROPPABLE));
		for (TskUSize i = index; i < index + count; i++) {
			tsk_trait_droppable_drop(
			    tsk_type_trait(array->element_type, TSK_TRAIT_DROPPABLE),
			    tsk_array_get(array, i)
			);
		}
	}

	memmove(
	    tsk_array_get(array, index),
	    tsk_array_get_const(array, index + count),
	    (tsk_array_length(array) - index - count) * tsk_trait_complete_size(tsk_type_trait(array->element_type, TSK_TRAIT_COMPLETE))
	);

	array->length -= count;
}
TskBool tsk_array_push_front(TskArray *array, TskAny *element) {
	assert(tsk_array_is_valid(array));
	assert(element != TSK_NULL);

	return tsk_array_insert(array, 0, element, 1);
}
TskBool tsk_array_push_back(TskArray *array, TskAny *element) {
	assert(tsk_array_is_valid(array));
	assert(element != TSK_NULL);

	return tsk_array_insert(array, tsk_array_length(array), element, 1);
}
TskBool tsk_array_pop_front(TskArray *array, TskAny *element) {
	assert(tsk_array_is_valid(array));

	if (tsk_array_is_empty(array)) {
		return TSK_FALSE;
	}

	tsk_array_remove(array, 0, element, 1);

	return TSK_TRUE;
}
TskBool tsk_array_pop_back(TskArray *array, TskAny *element) {
	assert(tsk_array_is_valid(array));

	if (tsk_array_is_empty(array)) {
		return TSK_FALSE;
	}

	tsk_array_remove(array, tsk_array_length(array) - 1, element, 1);

	return TSK_TRUE;
}
TskBool tsk_array_linear_search(const TskArray *array, const TskAny *element, TskUSize *index) {
	return tsk_array_view_const_linear_search(tsk_array_to_view_const(array), element, index);
}
TskBool tsk_array_binary_search(const TskArray *array, const TskAny *element, TskUSize *index) {
	return tsk_array_view_const_binary_search(tsk_array_to_view_const(array), element, index);
}
TskUSize tsk_array_lower_bound(const TskArray *array, const TskAny *element) {
	return tsk_array_view_const_lower_bound(tsk_array_to_view_const(array), element);
}
TskUSize tsk_array_upper_bound(const TskArray *array, const TskAny *element) {
	return tsk_array_view_const_upper_bound(tsk_array_to_view_const(array), element);
}
TskEmpty tsk_array_equal_range(const TskArray *array, const TskAny *element, TskUSize *start, TskUSize *end) {
	tsk_array_view_const_equal_range(tsk_array_to_view_const(array), element, start, end);
}
TskEmpty tsk_array_sort(TskArray *array) {
	tsk_array_view_sort(tsk_array_to_view(array));
}
TskBool tsk_array_is_sorted(const TskArray *array) {
	return tsk_array_view_const_is_sorted(tsk_array_to_view_const(array));
}
TskUSize tsk_array_sorted_until(const TskArray *array) {
	return tsk_array_view_const_sorted_until(tsk_array_to_view_const(array));
}
TskEmpty tsk_array_partition(TskArray *array, TskBool (*predicate)(const TskAny *element)) {
	tsk_array_view_partition(tsk_array_to_view(array), predicate);
}
TskBool tsk_array_is_partitioned(const TskArray *array, TskBool (*predicate)(const TskAny *element)) {
	return tsk_array_view_const_is_partitioned(tsk_array_to_view_const(array), predicate);
}
TskUSize tsk_array_partition_point(TskArray *array, TskBool (*predicate)(const TskAny *element)) {
	return tsk_array_view_partition_point(tsk_array_to_view(array), predicate);
}
TskEmpty tsk_array_reverse(TskArray *array) {
	tsk_array_view_reverse(tsk_array_to_view(array));
}
TskOrdering tsk_array_compare(const TskArray *array_1, const TskArray *array_2) {
	return tsk_array_view_const_compare(
	    tsk_array_to_view_const(array_1),
	    tsk_array_to_view_const(array_2)
	);
}
TskBool tsk_array_equals(const TskArray *array_1, const TskArray *array_2) {
	return tsk_array_view_const_equals(
	    tsk_array_to_view_const(array_1),
	    tsk_array_to_view_const(array_2)
	);
}
TskEmpty tsk_array_hash(const TskArray *array, const TskTraitHasher *trait_hasher, TskAny *hasher) {
	tsk_array_view_const_hash(tsk_array_to_view_const(array), trait_hasher, hasher);
}

static const TskTraitComplete tsk_array_trait_complete = {
	.size      = sizeof(TskArray),
	.alignment = alignof(TskArray),
};

static TskEmpty tsk_array_trait_droppable_drop(TskAny *array) {
	tsk_array_drop(array);
}
static const TskTraitDroppable tsk_array_trait_droppable = {
	.drop = tsk_array_trait_droppable_drop,
};

static TskBool tsk_array_trait_clonable_clone(const TskAny *array_1, TskAny *array_2) {
	return tsk_array_clone(array_1, array_2);
}
static const TskTraitClonable tsk_array_trait_clonable = {
	.trait_complete = &tsk_array_trait_complete,
	.clone          = tsk_array_trait_clonable_clone,
};

static TskOrdering tsk_array_trait_comparable_compare(const TskAny *array_1, const TskAny *array_2) {
	return tsk_array_compare(array_1, array_2);
}
static const TskTraitComparable tsk_array_trait_comparable = {
	.compare = tsk_array_trait_comparable_compare,
};

static TskBool tsk_array_type_equatable_equals(const TskAny *array_1, const TskAny *array_2) {
	return tsk_array_equals(array_1, array_2);
}
static const TskTraitEquatable tsk_array_trait_equatable = {
	.equals = tsk_array_type_equatable_equals,
};

static TskEmpty tsk_array_trait_hashable_hash(const TskAny *array, const TskTraitHasher *trait_hasher, TskAny *hasher) {
	tsk_array_hash(array, trait_hasher, hasher);
}
static const TskTraitHashable tsk_array_trait_hashable = {
	.hash = tsk_array_trait_hashable_hash,
};

const TskType tsk_array_type = {
	.name   = "TskArray",
	.traits = {
	    [TSK_TRAIT_COMPLETE]   = &tsk_array_trait_complete,
	    [TSK_TRAIT_DROPPABLE]  = &tsk_array_trait_droppable,
	    [TSK_TRAIT_CLONABLE]   = &tsk_array_trait_clonable,
	    [TSK_TRAIT_COMPARABLE] = &tsk_array_trait_comparable,
	    [TSK_TRAIT_EQUATABLE]  = &tsk_array_trait_equatable,
	    [TSK_TRAIT_HASHABLE]   = &tsk_array_trait_hashable,
	},
};

TskBool tsk_array_view_is_valid(TskArrayView array_view) {
	return array_view.elements != TSK_NULL && tsk_type_is_valid(array_view.element_type) &&
	       tsk_type_has_trait(array_view.element_type, TSK_TRAIT_COMPLETE) && (array_view.elements != TSK_NULL || array_view.length == 0) && array_view.length <= SIZE_MAX / tsk_trait_complete_size(tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPLETE));
}
TskArrayView tsk_array_view_slice(TskArrayView array_view, TskUSize start, TskUSize end, TskISize step) {
	assert(tsk_array_view_is_valid(array_view));
	assert(step != 0);

	if (step > 0) {
		assert(end <= tsk_array_view_length(array_view));
		assert(start <= end);

		TskArrayView array_view_slice = {
			.element_type = array_view.element_type,
			.elements     = (TskU8 *)array_view.elements + (start * tsk_trait_complete_size(tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPLETE))),
			.length       = (end - start + (TskUSize)step - 1) / (TskUSize)step,
			.stride       = array_view.stride * step,
		};

		assert(tsk_array_view_is_valid(array_view_slice));

		return array_view_slice;
	}

	assert(start <= tsk_array_view_length(array_view));
	assert(end <= start);

	TskArrayView array_view_slice = {
		.element_type = array_view.element_type,
		.elements     = (TskU8 *)array_view.elements + (start * tsk_trait_complete_size(tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPLETE))) - tsk_trait_complete_size(tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPLETE)),
		.length       = (start - end + (TskUSize)-step - 1) / (TskUSize)-step,
		.stride       = array_view.stride * step,
	};

	assert(tsk_array_view_is_valid(array_view_slice));

	return array_view_slice;
}
TskUSize tsk_array_view_length(TskArrayView array_view) {
	assert(tsk_array_view_is_valid(array_view));

	return array_view.length;
}
TskBool tsk_array_view_is_empty(TskArrayView array_view) {
	assert(tsk_array_view_is_valid(array_view));

	return tsk_array_view_length(array_view) == 0;
}
TskAny *tsk_array_view_get(TskArrayView array_view, TskUSize index) {
	assert(tsk_array_view_is_valid(array_view));
	assert(index < tsk_array_view_length(array_view));

	if (array_view.stride > 0) {
		return (TskU8 *)array_view.elements + (index * (TskUSize)array_view.stride * tsk_trait_complete_size(tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPLETE)));
	}

	return (TskU8 *)array_view.elements - (index * (TskUSize)-array_view.stride * tsk_trait_complete_size(tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPLETE)));
}
TskAny *tsk_array_view_front(TskArrayView array_view) {
	assert(tsk_array_view_is_valid(array_view));

	if (tsk_array_view_is_empty(array_view)) {
		return TSK_NULL;
	}

	return tsk_array_view_get(array_view, 0);
}
TskAny *tsk_array_view_back(TskArrayView array_view) {
	assert(tsk_array_view_is_valid(array_view));

	if (tsk_array_view_is_empty(array_view)) {
		return TSK_NULL;
	}

	return tsk_array_view_get(array_view, tsk_array_view_length(array_view) - 1);
}
TskEmpty tsk_array_view_swap(TskArrayView array_view, TskUSize index_1, TskUSize index_2) {
	assert(tsk_array_view_is_valid(array_view));
	assert(index_1 < tsk_array_view_length(array_view));
	assert(index_2 < tsk_array_view_length(array_view));

	if (index_1 == index_2) {
		return;
	}

	TskU8 *elements_1 = (TskU8 *)tsk_array_view_get(array_view, index_1);
	TskU8 *elements_2 = (TskU8 *)tsk_array_view_get(array_view, index_2);
	for (TskUSize i = 0; i < tsk_trait_complete_size(tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPLETE)); i++) {
		TskU8 byte    = elements_1[i];
		elements_1[i] = elements_2[i];
		elements_2[i] = byte;
	}
}
TskBool tsk_array_view_linear_search(TskArrayView array_view, const TskAny *element, TskUSize *index) {
	return tsk_array_view_const_binary_search(tsk_array_view_to_const(array_view), element, index);
}
TskBool tsk_array_view_binary_search(TskArrayView array_view, const TskAny *element, TskUSize *index) {
	return tsk_array_view_const_binary_search(tsk_array_view_to_const(array_view), element, index);
}
TskUSize tsk_array_view_lower_bound(TskArrayView array_view, const TskAny *element) {
	return tsk_array_view_const_lower_bound(tsk_array_view_to_const(array_view), element);
}
TskUSize tsk_array_view_upper_bound(TskArrayView array_view, const TskAny *element) {
	return tsk_array_view_const_upper_bound(tsk_array_view_to_const(array_view), element);
}
TskEmpty tsk_array_view_equal_range(TskArrayView array_view, const TskAny *element, TskUSize *start, TskUSize *end) {
	tsk_array_view_const_equal_range(tsk_array_view_to_const(array_view), element, start, end);
}
TskEmpty tsk_array_view_sort(TskArrayView array_view) {
	assert(tsk_array_view_is_valid(array_view));
	assert(tsk_type_has_trait(array_view.element_type, TSK_TRAIT_COMPARABLE));

	// PERF: find a way to sort, without allocating a buffer and copying the elements

	alignas(max_align_t) TskU8 buffer[1028];

	TskUSize elements_size = tsk_array_view_length(array_view) * tsk_trait_complete_size(tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPLETE));
	TskAny  *elements      = TSK_NULL;
	if (elements_size > sizeof(buffer)) {
		elements = malloc(elements_size);
		assert(elements != TSK_NULL);
	} else {
		elements = buffer;
	}

	for (TskUSize i = 0; i < tsk_array_view_length(array_view); i++) {
		memcpy(
		    (TskU8 *)elements + (i * tsk_trait_complete_size(tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPLETE))),
		    tsk_array_view_get(array_view, i),
		    tsk_trait_complete_size(tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPLETE))
		);
	}

	qsort(
	    elements,
	    tsk_array_view_const_length(tsk_array_view_to_const(array_view)),
	    tsk_trait_complete_size(tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPLETE)),
	    ((const TskTraitComparable *)tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPARABLE))->compare
	);

	for (TskUSize i = 0; i < tsk_array_view_length(array_view); i++) {
		memcpy(
		    tsk_array_view_get(array_view, i),
		    (TskU8 *)elements + (i * tsk_trait_complete_size(tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPLETE))),
		    tsk_trait_complete_size(tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPLETE))
		);
	}

	if (elements_size > sizeof(buffer)) {
		free(elements);
	}
}
TskBool tsk_array_view_is_sorted(TskArrayView array_view) {
	return tsk_array_view_const_is_sorted(tsk_array_view_to_const(array_view));
}
TskUSize tsk_array_view_sorted_until(TskArrayView array_view) {
	return tsk_array_view_const_sorted_until(tsk_array_view_to_const(array_view));
}
TskEmpty tsk_array_view_partition(TskArrayView array_view, TskBool (*predicate)(const TskAny *element)) {
	assert(tsk_array_view_is_valid(array_view));
	assert(predicate != TSK_NULL);

	TskUSize i = 0;
	for (; i < tsk_array_view_length(array_view); i++) {
		if (!predicate(tsk_array_view_get(array_view, i))) {
			break;
		}
	}

	for (TskUSize j = i + 1; j < tsk_array_view_length(array_view); j++) {
		if (predicate(tsk_array_view_get(array_view, j))) {
			tsk_array_view_swap(array_view, i, j);
			i++;
		}
	}
}
TskBool tsk_array_view_is_partitioned(TskArrayView array_view, TskBool (*predicate)(const TskAny *element)) {
	return tsk_array_view_const_is_partitioned(tsk_array_view_to_const(array_view), predicate);
}
TskUSize tsk_array_view_partition_point(TskArrayView array_view, TskBool (*predicate)(const TskAny *element)) {
	return tsk_array_view_const_partition_point(tsk_array_view_to_const(array_view), predicate);
}
TskEmpty tsk_array_view_reverse(TskArrayView array_view) {
	assert(tsk_array_view_is_valid(array_view));

	for (TskUSize i = 0; i < tsk_array_view_length(array_view) / 2; i++) {
		tsk_array_view_swap(array_view, i, tsk_array_view_length(array_view) - 1 - i);
	}
}
TskOrdering tsk_array_view_compare(TskArrayView array_view_1, TskArrayView array_view_2) {
	return tsk_array_view_const_compare(
	    tsk_array_view_to_const(array_view_1),
	    tsk_array_view_to_const(array_view_2)
	);
}
TskBool tsk_array_view_equals(TskArrayView array_view_1, TskArrayView array_view_2) {
	return tsk_array_view_const_equals(
	    tsk_array_view_to_const(array_view_1),
	    tsk_array_view_to_const(array_view_2)
	);
}
TskEmpty tsk_array_view_hash(TskArrayView array_view, const TskTraitHasher *trait_hasher, TskAny *hasher) {
	tsk_array_view_const_hash(tsk_array_view_to_const(array_view), trait_hasher, hasher);
}

static const TskTraitComplete tsk_array_view_trait_complete = {
	.size      = sizeof(TskArrayView),
	.alignment = alignof(TskArrayView),
};

static const TskTraitDroppable tsk_array_view_trait_droppable = {
	.drop = TSK_NULL,
};

static const TskTraitClonable tsk_array_view_trait_clonable = {
	.trait_complete = &tsk_array_view_trait_complete,
	.clone          = TSK_NULL,
};

static TskOrdering tsk_array_view_trait_comparable_compare(const TskAny *array_view_1, const TskAny *array_view_2) {
	return tsk_array_view_compare(*(const TskArrayView *)array_view_1, *(const TskArrayView *)array_view_2);
}
static const TskTraitComparable tsk_array_view_trait_comparable = {
	.compare = tsk_array_view_trait_comparable_compare,
};

static TskBool tsk_array_view_type_equatable_equals(const TskAny *array_view_1, const TskAny *array_view_2) {
	return tsk_array_view_equals(*(const TskArrayView *)array_view_1, *(const TskArrayView *)array_view_2);
}
static const TskTraitEquatable tsk_array_view_trait_equatable = {
	.equals = tsk_array_view_type_equatable_equals,
};

static TskEmpty tsk_array_view_trait_hashable_hash(const TskAny *array_view, const TskTraitHasher *trait_hasher, TskAny *hasher) {
	tsk_array_view_hash(*(const TskArrayView *)array_view, trait_hasher, hasher);
}
static const TskTraitHashable tsk_array_view_trait_hashable = {
	.hash = tsk_array_view_trait_hashable_hash,
};

const TskType tsk_array_view_type = {
	.name   = "TskArrayView",
	.traits = {
	    [TSK_TRAIT_COMPLETE]   = &tsk_array_view_trait_complete,
	    [TSK_TRAIT_DROPPABLE]  = &tsk_array_view_trait_droppable,
	    [TSK_TRAIT_CLONABLE]   = &tsk_array_view_trait_clonable,
	    [TSK_TRAIT_COMPARABLE] = &tsk_array_view_trait_comparable,
	    [TSK_TRAIT_EQUATABLE]  = &tsk_array_view_trait_equatable,
	    [TSK_TRAIT_HASHABLE]   = &tsk_array_view_trait_hashable,
	},
};

TskBool tsk_array_view_const_is_valid(TskArrayViewConst array_view) {
	return array_view.elements != TSK_NULL && tsk_type_is_valid(array_view.element_type) &&
	       tsk_type_has_trait(array_view.element_type, TSK_TRAIT_COMPLETE) && (array_view.elements != TSK_NULL || array_view.length == 0) && array_view.length <= SIZE_MAX / tsk_trait_complete_size(tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPLETE));
}
TskArrayViewConst tsk_array_view_const_slice(TskArrayViewConst array_view, TskUSize start, TskUSize end, TskISize step) {
	assert(tsk_array_view_const_is_valid(array_view));
	assert(step != 0);

	if (step > 0) {
		assert(end <= tsk_array_view_const_length(array_view));
		assert(start <= end);

		TskArrayViewConst array_view_slice = {
			.element_type = array_view.element_type,
			.elements     = (const TskU8 *)array_view.elements + (start * tsk_trait_complete_size(tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPLETE))),
			.length       = (end - start + (TskUSize)step - 1) / (TskUSize)step,
			.stride       = array_view.stride * step,
		};

		assert(tsk_array_view_const_is_valid(array_view_slice));

		return array_view_slice;
	}

	assert(start <= tsk_array_view_const_length(array_view));
	assert(end <= start);

	TskArrayViewConst array_view_slice = {
		.element_type = array_view.element_type,
		.elements     = (const TskU8 *)array_view.elements + (start * tsk_trait_complete_size(tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPLETE))) - tsk_trait_complete_size(tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPLETE)),
		.length       = (start - end + (TskUSize)-step - 1) / (TskUSize)-step,
		.stride       = array_view.stride * step,
	};

	assert(tsk_array_view_const_is_valid(array_view_slice));

	return array_view_slice;
}
TskUSize tsk_array_view_const_length(TskArrayViewConst array_view) {
	assert(tsk_array_view_const_is_valid(array_view));

	return array_view.length;
}
TskBool tsk_array_view_const_is_empty(TskArrayViewConst array_view) {
	assert(tsk_array_view_const_is_valid(array_view));

	return tsk_array_view_const_length(array_view) == 0;
}
const TskAny *tsk_array_view_const_get(TskArrayViewConst array_view, TskUSize index) {
	assert(tsk_array_view_const_is_valid(array_view));
	assert(index < tsk_array_view_const_length(array_view));

	if (array_view.stride > 0) {
		return (const TskU8 *)array_view.elements + (index * (TskUSize)array_view.stride * tsk_trait_complete_size(tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPLETE)));
	}

	return (const TskU8 *)array_view.elements - (index * (TskUSize)-array_view.stride * tsk_trait_complete_size(tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPLETE)));
}
const TskAny *tsk_array_view_const_front(TskArrayViewConst array_view) {
	assert(tsk_array_view_const_is_valid(array_view));

	if (tsk_array_view_const_is_empty(array_view)) {
		return TSK_NULL;
	}

	return tsk_array_view_const_get(array_view, 0);
}
const TskAny *tsk_array_view_const_back(TskArrayViewConst array_view) {
	assert(tsk_array_view_const_is_valid(array_view));

	if (tsk_array_view_const_is_empty(array_view)) {
		return TSK_NULL;
	}

	return tsk_array_view_const_get(array_view, tsk_array_view_const_length(array_view) - 1);
}
TskBool tsk_array_view_const_linear_search(TskArrayViewConst array_view, const TskAny *element, TskUSize *index) {
	assert(tsk_array_view_const_is_valid(array_view));
	assert(element != TSK_NULL);
	assert(tsk_type_has_trait(array_view.element_type, TSK_TRAIT_EQUATABLE));

	for (TskUSize i = 0; i < tsk_array_view_const_length(array_view); i++) {
		if (tsk_trait_equatable_equals(
		        tsk_type_trait(array_view.element_type, TSK_TRAIT_EQUATABLE),
		        tsk_array_view_const_get(array_view, i),
		        element
		    )) {
			if (index != TSK_NULL) {
				*index = i;
			}
			return TSK_TRUE;
		}
	}

	return TSK_FALSE;
}
TskBool tsk_array_view_const_binary_search(TskArrayViewConst array_view, const TskAny *element, TskUSize *index) {
	assert(tsk_array_view_const_is_valid(array_view));
	assert(element != TSK_NULL);
	assert(tsk_type_has_trait(array_view.element_type, TSK_TRAIT_COMPARABLE));

	TskUSize left  = 0;
	TskUSize right = tsk_array_view_const_length(array_view) - 1;
	while (left <= right) {
		TskUSize    middle   = left + ((right - left) / 2);
		TskOrdering ordering = tsk_trait_comparable_compare(
		    tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPARABLE),
		    tsk_array_view_const_get(array_view, middle),
		    element
		);
		switch (ordering) {
			case TSK_ORDERING_LESS: {
				left = middle + 1;
			} break;
			case TSK_ORDERING_EQUAL: {
				if (index != TSK_NULL) {
					*index = middle;
				}
				return TSK_TRUE;
			}
			case TSK_ORDERING_GREATER: {
				right = middle - 1;
			} break;
		}
	}

	if (index != TSK_NULL) {
		*index = left;
	}
	return TSK_FALSE;
}
TskUSize tsk_array_view_const_lower_bound(TskArrayViewConst array_view, const TskAny *element) {
	assert(tsk_array_view_const_is_valid(array_view));
	assert(element != TSK_NULL);
	assert(tsk_type_has_trait(array_view.element_type, TSK_TRAIT_COMPARABLE));

	TskUSize left  = 0;
	TskUSize right = tsk_array_view_const_length(array_view);
	TskUSize index = tsk_array_view_const_length(array_view);
	while (left < right) {
		TskUSize    middle   = left + ((right - left) / 2);
		TskOrdering ordering = tsk_trait_comparable_compare(
		    tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPARABLE),
		    tsk_array_view_const_get(array_view, middle),
		    element
		);
		switch (ordering) {
			case TSK_ORDERING_LESS: {
				left = middle + 1;
			} break;
			case TSK_ORDERING_EQUAL:
			case TSK_ORDERING_GREATER: {
				index = middle;
				right = middle;
			} break;
		}
	}

	return index;
}
TskUSize tsk_array_view_const_upper_bound(TskArrayViewConst array_view, const TskAny *element) {
	assert(tsk_array_view_const_is_valid(array_view));
	assert(element != TSK_NULL);
	assert(tsk_type_has_trait(array_view.element_type, TSK_TRAIT_COMPARABLE));

	TskUSize left  = 0;
	TskUSize right = tsk_array_view_const_length(array_view);
	TskUSize index = tsk_array_view_const_length(array_view);
	while (left < right) {
		TskUSize    middle   = left + ((right - left) / 2);
		TskOrdering ordering = tsk_trait_comparable_compare(
		    tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPARABLE),
		    tsk_array_view_const_get(array_view, middle),
		    element
		);
		switch (ordering) {
			case TSK_ORDERING_LESS:
			case TSK_ORDERING_EQUAL: {
				left = middle + 1;
			} break;
			case TSK_ORDERING_GREATER: {
				index = middle;
				right = middle;
			} break;
		}
	}

	return index;
}
TskEmpty tsk_array_view_const_equal_range(TskArrayViewConst array_view, const TskAny *element, TskUSize *start, TskUSize *end) {
	assert(tsk_array_view_const_is_valid(array_view));
	assert(element != TSK_NULL);
	assert(tsk_type_has_trait(array_view.element_type, TSK_TRAIT_COMPARABLE));
	assert(start != TSK_NULL);
	assert(end != TSK_NULL);

	*start = tsk_array_view_const_lower_bound(array_view, element);
	*end   = *start + tsk_array_view_const_upper_bound(
                      tsk_array_view_const_slice(
                          array_view,
                          *start,
                          tsk_array_view_const_length(array_view),
                          1
                      ),
                      element
                  );
}
TskBool tsk_array_view_const_is_sorted(TskArrayViewConst array_view) {
	return tsk_array_view_const_sorted_until(array_view) == tsk_array_view_const_length(array_view);
}
TskUSize tsk_array_view_const_sorted_until(TskArrayViewConst array_view) {
	assert(tsk_array_view_const_is_valid(array_view));
	assert(tsk_type_has_trait(array_view.element_type, TSK_TRAIT_COMPARABLE));

	for (TskUSize i = 1; i < tsk_array_view_const_length(array_view); i++) {
		TskOrdering ordering = tsk_trait_comparable_compare(
		    tsk_type_trait(array_view.element_type, TSK_TRAIT_COMPARABLE),
		    tsk_array_view_const_get(array_view, i - 1),
		    tsk_array_view_const_get(array_view, i)
		);
		if (ordering > 0) {
			return i;
		}
	}

	return tsk_array_view_const_length(array_view);
}
TskBool tsk_array_view_const_is_partitioned(TskArrayViewConst array_view, TskBool (*predicate)(const TskAny *element)) {
	TskUSize i = 0;
	for (; i < tsk_array_view_const_length(array_view); i++) {
		if (!predicate(tsk_array_view_const_get(array_view, i))) {
			break;
		}
	}

	for (i++; i < tsk_array_view_const_length(array_view); i++) {
		if (predicate(tsk_array_view_const_get(array_view, i))) {
			return TSK_FALSE;
		}
	}

	return TSK_TRUE;
}
TskUSize tsk_array_view_const_partition_point(TskArrayViewConst array_view, TskBool (*predicate)(const TskAny *element)) {
	assert(tsk_array_view_const_is_valid(array_view));
	assert(predicate != TSK_NULL);

	TskUSize left  = 0;
	TskUSize right = tsk_array_view_const_length(array_view) - 1;
	TskUSize index = tsk_array_view_const_length(array_view);
	while (left <= right) {
		TskUSize middle = left + ((right - left) / 2);
		if (predicate(tsk_array_view_const_get(array_view, middle))) {
			left = middle + 1;
		} else {
			index = middle;
			right = middle - 1;
		}
	}

	return index;
}
TskOrdering tsk_array_view_const_compare(TskArrayViewConst array_view_1, TskArrayViewConst array_view_2) {
	assert(tsk_array_view_const_is_valid(array_view_1));
	assert(tsk_array_view_const_is_valid(array_view_2));
	assert(array_view_1.element_type == array_view_2.element_type);
	assert(tsk_type_has_trait(array_view_1.element_type, TSK_TRAIT_COMPARABLE));

	for (TskUSize i = 0; i < tsk_array_view_const_length(array_view_1) && i < tsk_array_view_const_length(array_view_2); i++) {
		TskOrdering ordering = tsk_trait_comparable_compare(
		    tsk_type_trait(array_view_1.element_type, TSK_TRAIT_COMPARABLE),
		    tsk_array_view_const_get(array_view_1, i),
		    tsk_array_view_const_get(array_view_2, i)
		);
		if (ordering != 0) {
			return ordering;
		}
	}

	return (tsk_array_view_const_length(array_view_1) > tsk_array_view_const_length(array_view_2)) -
	       (tsk_array_view_const_length(array_view_1) < tsk_array_view_const_length(array_view_2));
}
TskBool tsk_array_view_const_equals(TskArrayViewConst array_view_1, TskArrayViewConst array_view_2) {
	assert(tsk_array_view_const_is_valid(array_view_1));
	assert(tsk_array_view_const_is_valid(array_view_2));
	assert(array_view_1.element_type == array_view_2.element_type);
	assert(tsk_type_has_trait(array_view_1.element_type, TSK_TRAIT_EQUATABLE));

	if (tsk_array_view_const_length(array_view_1) != tsk_array_view_const_length(array_view_2)) {
		return TSK_FALSE;
	}

	for (TskUSize i = 0; i < tsk_array_view_const_length(array_view_1); i++) {
		if (!tsk_trait_equatable_equals(
		        tsk_type_trait(array_view_1.element_type, TSK_TRAIT_EQUATABLE),
		        tsk_array_view_const_get(array_view_1, i),
		        tsk_array_view_const_get(array_view_2, i)
		    )) {
			return TSK_FALSE;
		}
	}

	return TSK_TRUE;
}
TskEmpty tsk_array_view_const_hash(TskArrayViewConst array_view, const TskTraitHasher *trait_hasher, TskAny *hasher) {
	assert(tsk_array_view_const_is_valid(array_view));
	assert(tsk_type_has_trait(array_view.element_type, TSK_TRAIT_HASHABLE));

	tsk_usize_hash(tsk_array_view_const_length(array_view), trait_hasher, hasher);
	for (TskUSize i = 0; i < tsk_array_view_const_length(array_view); i++) {
		tsk_trait_hashable_hash(
		    tsk_type_trait(array_view.element_type, TSK_TRAIT_HASHABLE),
		    tsk_array_view_const_get(array_view, i),
		    trait_hasher,
		    hasher
		);
	}
}

static const TskTraitComplete tsk_array_view_const_trait_complete = {
	.size      = sizeof(TskArrayViewConst),
	.alignment = alignof(TskArrayViewConst),
};

static const TskTraitDroppable tsk_array_view_const_trait_droppable = {
	.drop = TSK_NULL,
};

static const TskTraitClonable tsk_array_view_const_trait_clonable = {
	.trait_complete = &tsk_array_view_const_trait_complete,
	.clone          = TSK_NULL,
};

static TskOrdering tsk_array_view_const_trait_comparable_compare(const TskAny *array_view_1, const TskAny *array_view_2) {
	return tsk_array_view_const_compare(*(const TskArrayViewConst *)array_view_1, *(const TskArrayViewConst *)array_view_2);
}
static const TskTraitComparable tsk_array_view_const_trait_comparable = {
	.compare = tsk_array_view_const_trait_comparable_compare,
};

static TskBool tsk_array_view_const_type_equatable_equals(const TskAny *array_view_1, const TskAny *array_view_2) {
	return tsk_array_view_const_equals(*(const TskArrayViewConst *)array_view_1, *(const TskArrayViewConst *)array_view_2);
}
static const TskTraitEquatable tsk_array_view_const_trait_equatable = {
	.equals = tsk_array_view_const_type_equatable_equals,
};

static TskEmpty tsk_array_view_const_trait_hashable_hash(const TskAny *array_view, const TskTraitHasher *trait_hasher, TskAny *hasher) {
	tsk_array_view_const_hash(*(const TskArrayViewConst *)array_view, trait_hasher, hasher);
}
static const TskTraitHashable tsk_array_view_const_trait_hashable = {
	.hash = tsk_array_view_const_trait_hashable_hash,
};

const TskType tsk_array_view_const_type = {
	.name   = "TskArrayViewConst",
	.traits = {
	    [TSK_TRAIT_COMPLETE]   = &tsk_array_view_const_trait_complete,
	    [TSK_TRAIT_DROPPABLE]  = &tsk_array_view_const_trait_droppable,
	    [TSK_TRAIT_CLONABLE]   = &tsk_array_view_const_trait_clonable,
	    [TSK_TRAIT_COMPARABLE] = &tsk_array_view_const_trait_comparable,
	    [TSK_TRAIT_EQUATABLE]  = &tsk_array_view_const_trait_equatable,
	    [TSK_TRAIT_HASHABLE]   = &tsk_array_view_const_trait_hashable,
	},
};

TskArrayView tsk_array_to_view(TskArray *array) {
	assert(tsk_array_is_valid(array));

	TskArrayView array_view = {
		.element_type = array->element_type,
		.elements     = array->elements,
		.length       = array->length,
		.stride       = 1,
	};

	assert(tsk_array_view_is_valid(array_view));

	return array_view;
}
TskArrayViewConst tsk_array_to_view_const(const TskArray *array) {
	assert(tsk_array_is_valid(array));

	TskArrayViewConst array_view = {
		.element_type = array->element_type,
		.elements     = array->elements,
		.length       = array->length,
		.stride       = 1,
	};

	assert(tsk_array_view_const_is_valid(array_view));

	return array_view;
}

TskArrayViewConst tsk_array_view_to_const(TskArrayView array_view) {
	assert(tsk_array_view_is_valid(array_view));

	TskArrayViewConst array_view_const = {
		.element_type = array_view.element_type,
		.elements     = array_view.elements,
		.length       = array_view.length,
		.stride       = array_view.stride,
	};

	assert(tsk_array_view_const_is_valid(array_view_const));

	return array_view_const;
}
