#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "mem/ring.h"

/* A test case that does nothing and succeeds. */
static void test_peek_and_enqueue(void **state)
{
    size_t num = 10;

    MemRing ring = mem_ring_alloc(sizeof(num) * 10);

    // Test peeking without anything enqueued
    void* peek_addr = 0;
    bool ok = mem_ring_peek(&ring, &peek_addr, sizeof(num));
    assert_false(ok); // should return false
    assert_ptr_equal(0, peek_addr); // pointer should be unchanged

    // Test dequeuing with empty ring
    size_t dequeued = 42;
    ok = mem_ring_dequeue(&ring, &dequeued, sizeof(dequeued));
    assert_false(ok);
    assert_int_equal(42, dequeued);

    // Enqueue 10
    mem_ring_enqueue(&ring, &num, sizeof(num));

    // Peeking 10 should work
    ok = mem_ring_peek(&ring, &peek_addr, sizeof(num));
    assert_true(ok);
    assert_ptr_not_equal(0, peek_addr); 
    assert_int_equal(num, *((size_t*) peek_addr));

    // Dequeuing should also work
    dequeued = 7;
    ok = mem_ring_dequeue(&ring, &dequeued, sizeof(dequeued));
    assert_true(ok);
    assert_int_equal(num, dequeued);

    // But not on an empty ring
    dequeued = 42;
    ok = mem_ring_dequeue(&ring, &dequeued, sizeof(dequeued));
    assert_false(ok);
    assert_int_equal(42, dequeued);

    mem_ring_free(&ring);
}

static void test_overflow(void **state)
{
    const int units = 10;
    MemRing ring = mem_ring_alloc(sizeof(int) * units);

    for(int i = 0; i < units; ++i) {
        bool ok = mem_ring_enqueue(&ring, &i, sizeof(int));
        assert_true(ok);
    }

    int num = units;
    bool ok = mem_ring_enqueue(&ring, &num, sizeof(int));
    assert_false(ok); // cannot enqueue anymore, should fail

    // this should not affect dequeuing, which should work normally
    for(int i = 0; i < units; ++i) {
        int dequeued = -31;
        bool ok = mem_ring_dequeue(&ring, &dequeued, sizeof(int));
        assert_true(ok);
        assert_int_equal(i, dequeued);
    }

    mem_ring_free(&ring);
}

static void test_wrapping(void **state)
{
    MemRing ring = mem_ring_alloc(sizeof(int) * 10);
    
    // Enqueue five numbers
    int num = 42;
    bool ok = mem_ring_enqueue(&ring, &num, sizeof(int));
    assert_true(ok);
    ok = mem_ring_enqueue(&ring, &num, sizeof(int));
    assert_true(ok);
    ok = mem_ring_enqueue(&ring, &num, sizeof(int));
    assert_true(ok);
    ok = mem_ring_enqueue(&ring, &num, sizeof(int));
    assert_true(ok);
    ok = mem_ring_enqueue(&ring, &num, sizeof(int));
    assert_true(ok);

    // And dequeue three
    num = -47;
    ok = mem_ring_dequeue(&ring, &num, sizeof(int));
    assert_true(ok);
    assert_int_equal(42, num);
    ok = mem_ring_dequeue(&ring, &num, sizeof(int));
    assert_true(ok);
    assert_int_equal(42, num);
    ok = mem_ring_dequeue(&ring, &num, sizeof(int));
    assert_true(ok);
    assert_int_equal(42, num);

    // Now enqueue eight, which should make the ring full, wrapping
    // over the highest index, so the newest index is one lower than the oldest
    // somewhere in the middle of the ring
    for(int i = 0; i < 8; ++i) {
        ok = mem_ring_enqueue(&ring, &i, sizeof(int));
        assert_true(ok);
    }

    // more enqueuing should fail
    ok = mem_ring_enqueue(&ring, &num, sizeof(int));
    assert_false(ok);

    // Dequeue the remaining two 42s
    num = 1000;
    ok = mem_ring_dequeue(&ring, &num, sizeof(int));
    assert_true(ok);
    assert_int_equal(42, num);
    ok = mem_ring_dequeue(&ring, &num, sizeof(int));
    assert_true(ok);
    assert_int_equal(42, num);
    
    // Now there should be the eight wrapping over the highest index
    for(int i = 0; i < 8; ++i) {
        int num = -5555;
        ok = mem_ring_dequeue(&ring, &num, sizeof(int));
        assert_true(ok);
        assert_int_equal(i, num);
    }

    mem_ring_free(&ring);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_peek_and_enqueue),
        cmocka_unit_test(test_overflow),
        cmocka_unit_test(test_wrapping)
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
