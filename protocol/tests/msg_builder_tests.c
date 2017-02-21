#include "msg/builder.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>

static void test_borrow(void **state)
{
    MsgBuilder builder;
    uint8_t frame_length = 42;
    uint8_t buffer_length = 24;

    msg_builder_init(&builder);
    MemBlock* msg = msg_builder_borrow(&builder, frame_length, buffer_length);

    uint8_t* msg_data = (uint8_t*) msg->data;
    assert_int_equal(msg->size, 7);
    assert_int_equal(msg_data[0], 0); // message type for borrow is 0
    assert_int_equal(msg_data[1], 0); // message ID least significant byte is 0
    assert_int_equal(msg_data[2], 0); // message ID most significant byte is 0
    assert_int_equal(msg_data[3], 2); // payload length least significant byte is 2
    assert_int_equal(msg_data[4], 0); // payload length most significant byte is 0
    assert_int_equal(msg_data[5], frame_length); // first payload byte is frame length
    assert_int_equal(msg_data[6], buffer_length); // second payload byte is buffer length

    msg_builder_free(&builder);
}

static void test_lent(void **state)
{
    MsgBuilder builder;

    msg_builder_init(&builder);
    MemBlock* msg = msg_builder_lent(&builder);

    uint8_t* msg_data = (uint8_t*) msg->data;
    assert_int_equal(msg->size, 5);
    assert_int_equal(msg_data[0], 1); // message type for lent is 1
    assert_int_equal(msg_data[1], 0); // message ID least significant byte is 0
    assert_int_equal(msg_data[2], 0); // message ID most significant byte is 0
    assert_int_equal(msg_data[3], 0); // payload length least significant byte is 0
    assert_int_equal(msg_data[4], 0); // payload length most significant byte is 0

    msg_builder_free(&builder);
}

static void test_enqueue(void **state)
{
    MsgBuilder builder;
    uint8_t frame[] = {
        1, 2, 3,
        4, 5, 6
    };
    size_t frame_len = sizeof(frame) / sizeof(uint8_t);

    msg_builder_init(&builder);
    MemBlock* msg_block = msg_builder_enqueue(&builder, frame, frame_len);

    uint8_t* msg = (uint8_t*) msg_block->data;
    assert_int_equal(msg_block->size, (5 + 6)); // 5 bytes header, 6 bytes payload
    assert_int_equal(msg[0], 2); // message type for enqueue is 2
    assert_int_equal(msg[1], 0); // message ID least significant byte is 0
    assert_int_equal(msg[2], 0); // message ID most significant byte is 0
    assert_int_equal(msg[3], 6); // payload length least significant byte is 0
    assert_int_equal(msg[4], 0); // payload length most significant byte is 0

    assert_int_equal(msg[5], 1);
    assert_int_equal(msg[6], 2);
    assert_int_equal(msg[7], 3);

    assert_int_equal(msg[8], 4);
    assert_int_equal(msg[9], 5);
    assert_int_equal(msg[10], 6);

    msg_builder_free(&builder);
}

static void test_fail(void **state)
{
    MsgBuilder builder;
    uint16_t causing_message_id = 0xB1B2;
    uint8_t error_code = 7;

    msg_builder_init(&builder);
    MemBlock* msg_block = msg_builder_fail(&builder, causing_message_id, error_code);

    uint8_t* msg = (uint8_t*) msg_block->data;
    assert_int_equal(msg_block->size, 8);
    assert_int_equal(msg[0], 255); // message type for lent is 255
    assert_int_equal(msg[1], 0); // message ID least significant byte is 0
    assert_int_equal(msg[2], 0); // message ID most significant byte is 0
    assert_int_equal(msg[3], 3); // payload length least significant byte is 3
    assert_int_equal(msg[4], 0); // payload length most significant byte is 0
    assert_int_equal(msg[5], 0xB2); // causing message id lsb
    assert_int_equal(msg[6], 0xB1); // causing message id msb
    assert_int_equal(msg[7], 7); // error code

    msg_builder_free(&builder);
}

static void test_msg_id_overflow(void **state)
{
    MsgBuilder builder;

    msg_builder_init(&builder);

    for(int i = 0; i < 65535; ++i)
    {
        msg_builder_lent(&builder);
    }

    MemBlock* msg_block = msg_builder_lent(&builder);
    uint8_t* msg = (uint8_t*) msg_block->data;
    assert_int_equal(msg[1], 255); // message ID least significant byte is 255
    assert_int_equal(msg[2], 255); // message ID most significant byte is 255

    msg_block = msg_builder_lent(&builder);
    msg = (uint8_t*) msg_block->data;
    assert_int_equal(msg[1], 0); // message ID least significant byte is 0
    assert_int_equal(msg[2], 0); // message ID most significant byte is 0

    msg_block = msg_builder_lent(&builder);
    msg = (uint8_t*) msg_block->data;
    assert_int_equal(msg[1], 1); // message ID least significant byte is 1
    assert_int_equal(msg[2], 0); // message ID most significant byte is 0

    msg_builder_free(&builder);
}

static void test_reallocations(void **state)
{
    MsgBuilder builder;

    msg_builder_init(&builder);
    MemBlock* old_msg = msg_builder_lent(&builder);
    void* old_msg_data = old_msg->data;

    void* some_frame = malloc(1024 * 10);
    MemBlock* new_msg = msg_builder_enqueue(&builder, some_frame, 1024 * 10);

    // points to the same internal MemBlock, therefore the old message changed (inteneded behavior)
    assert_ptr_equal(old_msg->data, new_msg->data);

    // The previously saved data pointer now points to invalid memory and is different from the newly allocated memory
    assert_true((old_msg_data != old_msg->data && old_msg_data != new_msg->data));

    // The previously saved data pointer now points to invalid memory and is different from the newly
    // allocated memory that is now referenced by moth MemBlock pointers
    assert_ptr_not_equal(old_msg_data, old_msg->data);
    assert_ptr_not_equal(old_msg_data, new_msg->data);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_borrow),
        cmocka_unit_test(test_lent),
        cmocka_unit_test(test_enqueue),
        cmocka_unit_test(test_fail),
        cmocka_unit_test(test_msg_id_overflow),
        cmocka_unit_test(test_reallocations)
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
