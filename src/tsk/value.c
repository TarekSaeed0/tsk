#include <tsk/value.h>

#include <assert.h>
#include <stdlib.h>

TskBool tsk_value_is_valid(const TskValue *value) {
	return value != TSK_NULL &&
	       tsk_type_is_valid(value->type) &&
	       tsk_type_has_trait(value->type, TSK_TRAIT_COMPLETE) &&
	       (tsk_trait_complete_size(tsk_type_trait(value->type, TSK_TRAIT_COMPLETE)) <= sizeof(value->data.stack_allocated) || value->data.heap_allocated != TSK_NULL);
}
TskBool tsk_value_new(TskValue *value, const TskType *type) {
	assert(value != TSK_NULL);
	assert(tsk_type_is_valid(type));
	assert(tsk_type_has_trait(type, TSK_TRAIT_COMPLETE));

	if (tsk_trait_complete_size(tsk_type_trait(type, TSK_TRAIT_COMPLETE)) > sizeof(value->data.stack_allocated)) {
		TskAny *heap_allocated = malloc(tsk_trait_complete_size(tsk_type_trait(type, TSK_TRAIT_COMPLETE)));
		if (heap_allocated == TSK_NULL) {
			return TSK_FALSE;
		}

		value->data.heap_allocated = heap_allocated;
	}

	value->type = type;

	return TSK_TRUE;
}
TskEmpty tsk_value_drop(TskValue *value) {
	assert(tsk_value_is_valid(value));

	tsk_trait_droppable_drop(
	    tsk_type_trait(value->type, TSK_TRAIT_DROPPABLE),
	    tsk_value_data(value)
	);

	if (tsk_trait_complete_size(tsk_type_trait(value->type, TSK_TRAIT_COMPLETE)) > sizeof(value->data.stack_allocated)) {
		free(value->data.heap_allocated);
		value->data.heap_allocated = TSK_NULL;
	}
}
const TskType *tsk_value_type(const TskValue *value) {
	assert(tsk_value_is_valid(value));

	return value->type;
}
TskAny *tsk_value_data(TskValue *value) {
	assert(tsk_value_is_valid(value));

	if (tsk_trait_complete_size(tsk_type_trait(value->type, TSK_TRAIT_COMPLETE)) > sizeof(value->data.stack_allocated)) {
		return value->data.heap_allocated;
	}

	return value->data.stack_allocated;
}
const TskAny *tsk_value_data_const(const TskValue *value) {
	assert(tsk_value_is_valid(value));

	if (tsk_trait_complete_size(tsk_type_trait(value->type, TSK_TRAIT_COMPLETE)) > sizeof(value->data.stack_allocated)) {
		return value->data.heap_allocated;
	}

	return value->data.stack_allocated;
}
