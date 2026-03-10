#include <tsk/deque.h>

#include <stdio.h>

int main(void) {
	TskDeque deque = tsk_deque_new(tsk_deque_type(tsk_i32_type));

	for (TskI32 i = 0; i < 10; i++) {
		tsk_deque_push_front(tsk_deque_type(tsk_i32_type), &deque, &(TskI32){ i });

		for (TskUSize j = 0; j < tsk_deque_length(tsk_deque_type(tsk_i32_type), &deque); j++) {
			printf("%p ", tsk_deque_get_const(tsk_deque_type(tsk_i32_type), &deque, j));
		}
		printf("\n");
	}

	for (TskUSize i = 0; i < tsk_deque_length(tsk_deque_type(tsk_i32_type), &deque); i++) {
		printf("%d ", *(const TskI32 *)tsk_deque_get_const(tsk_deque_type(tsk_i32_type), &deque, i));
	}
	printf("\n");

	tsk_deque_drop(tsk_deque_type(tsk_i32_type), &deque);
}
