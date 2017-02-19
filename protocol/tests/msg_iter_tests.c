#include "msg/iter.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

static uint8_t borrow_and_enqueue_msg_buf[] = {
  0,   // message type 0 = borrow
  0, 0, // order number is 0
  2, 0, // payload length is 2
  16,  // frame length 16ms
  200,  // buffer length 200

  2,   // message type 2 = enqueue
  1, 0, // order number is 1
  6, 0, // payload length is 6
  255, 0, 0,  // red
  0, 0, 255  // blue
};

static uint8_t all_types_msg_buf[] = {
    0,   // message type 0 = borrow
    0, 0, // order number is 0
    2, 0, // payload length is 2
    16,  // frame length 16ms
    200,  // buffer length 200

    1,   // message type 1 = lent
    1, 0, // order number is 1
    0, 0, // payload length is 0

    2,   // message type 2 = enqueue
    2, 0, // order number is 2
    6, 0, // payload length is 6
    255, 0, 0,  // red
    0, 0, 255,  // blue

    255,   // message type 255 = fail
    3, 0, // order number is 3
    3, 0, // payload length is 3
    24, 0,  // offending message id is 24
    42  // error code is 42
};



static void test_has_msg(void **state)
{
    MsgIter iter = msg_iter_make(
        borrow_and_enqueue_msg_buf,
        sizeof(borrow_and_enqueue_msg_buf)
    );

    assert_true(msg_iter_has_msg(&iter));

    msg_iter_next(&iter);

    assert_true(msg_iter_has_msg(&iter));

    msg_iter_next(&iter);

    assert_false(msg_iter_has_msg(&iter));

}

static void test_next(void **state)
{
    MsgIter iter = msg_iter_make(
        borrow_and_enqueue_msg_buf,
        sizeof(borrow_and_enqueue_msg_buf)
    );

    uint8_t first_msg_type = msg_iter_type(&iter);

    msg_iter_next(&iter);

    uint8_t second_msg_type = msg_iter_type(&iter);

    assert_true(first_msg_type != second_msg_type);

    msg_iter_next(&iter);

    expect_assert_failure(msg_iter_next(&iter));
}

static void test_type(void **state)
{
    MsgIter iter = msg_iter_make(
        all_types_msg_buf,
        sizeof(all_types_msg_buf)
    );

    assert_int_equal(msg_iter_type(&iter), MSG_TYPE_BORROW);
    msg_iter_next(&iter);
    assert_int_equal(msg_iter_type(&iter), MSG_TYPE_LENT);
    msg_iter_next(&iter);
    assert_int_equal(msg_iter_type(&iter), MSG_TYPE_ENQUEUE);
    msg_iter_next(&iter);
    assert_int_equal(msg_iter_type(&iter), MSG_TYPE_FAIL);
}

static void test_msg_iter_msg_id(void **state)
{
    MsgIter iter = msg_iter_make(
        borrow_and_enqueue_msg_buf,
        sizeof(borrow_and_enqueue_msg_buf)
    );

    assert_int_equal(msg_iter_msg_id(&iter), 0);

    msg_iter_next(&iter);

    assert_int_equal(msg_iter_msg_id(&iter), 1);
}


static void test_msg_iter_borrow_frame_length(void **state)
{
    MsgIter iter = msg_iter_make(
        borrow_and_enqueue_msg_buf,
        sizeof(borrow_and_enqueue_msg_buf)
    );

    assert_int_equal(msg_iter_borrow_frame_length(&iter), 16);

}

static void test_msg_iter_borrow_buffer_length(void **state)
{
    MsgIter iter = msg_iter_make(
        borrow_and_enqueue_msg_buf,
        sizeof(borrow_and_enqueue_msg_buf)
    );

    assert_int_equal(msg_iter_borrow_buffer_length(&iter), 200);

}

static void test_msg_iter_enqueue_frame(void **state)
{
    MsgIter iter = msg_iter_make(
        borrow_and_enqueue_msg_buf,
        sizeof(borrow_and_enqueue_msg_buf)
    );

    msg_iter_next(&iter);

    MemBlock frame = msg_iter_enqueue_frame(&iter);
    assert_int_equal(frame.size, frame.capacity);
    assert_int_equal(frame.size, 6);

    uint8_t* color1 = (uint8_t*) frame.data;
    assert_int_equal(color1[0], 255);
    assert_int_equal(color1[1], 0);
    assert_int_equal(color1[2], 0);

    uint8_t* color2 = color1 + 3;
    assert_int_equal(color2[0], 0);
    assert_int_equal(color2[1], 0);
    assert_int_equal(color2[2], 255);
}

static void test_fail(void** state)
{
    MsgIter iter = msg_iter_make(
        all_types_msg_buf,
        sizeof(all_types_msg_buf)
    );
    
    msg_iter_next(&iter);
    msg_iter_next(&iter);
    msg_iter_next(&iter);
    
    assert_int_equal(msg_iter_fail_error_code(&iter), 42);
    assert_int_equal(msg_iter_fail_offending_msg_id(&iter), 24);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_has_msg),
        cmocka_unit_test(test_next),
        cmocka_unit_test(test_type),
        cmocka_unit_test(test_msg_iter_msg_id),
        cmocka_unit_test(test_msg_iter_borrow_frame_length),
        cmocka_unit_test(test_msg_iter_borrow_buffer_length),
        cmocka_unit_test(test_msg_iter_enqueue_frame),
        cmocka_unit_test(test_fail)

    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
