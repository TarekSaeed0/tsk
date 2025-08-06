#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <cmocka.h>

static void test_tsk(void **state) {
	(void)state;
}

int main(void) {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_tsk),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
