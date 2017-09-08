#include "time/now.h"
#include "time/sleep.h"

extern "C" {
    #include <stdarg.h>
    #include <stddef.h>
    #include <setjmp.h>
    #include <cmocka.h>
}

#include <stdio.h>

static void test_delta(void **state)
{
    const int tolerance = 10; // 5ms more or less is ok
    const int chosen_delta = 200;

    int time_before = time_now();
    time_sleep(chosen_delta);
    int time_after = time_now();

    int delta = time_after - time_before;

    assert_in_range(delta,
                    chosen_delta - tolerance,
                    chosen_delta + tolerance);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_delta),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
