#include "atolla/source.h"
#include "msg/builder.h"
#include "msg/iter.h"
#include "udp_socket/udp_socket.h"
#include "time/sleep.h"
#include "time/now.h"
#include "test/assert.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

/**
 * A time in milliseconds to wait in order to wait for data to be sent through local
 * loopback and arrive at the other local end.
 */
const int loopback_send_time_ms = 5;
const int frame_ms = 17;
const int buffered_frame_count = 60;

/**
 * Initializes a test by creating a source as well as a socket that simulates the sink,
 * along with a builder for easy creation of messages in the tests.
 *
 * Upon return, the source is in state ATOLLA_SOURCE_STATE_OPEN.
 */
static void setup_test(AtollaSource *source, UdpSocket* sink_socket, MsgBuilder* builder)
{
    const size_t test_ports_len = 12;
    // Use different ports so we do not have to wait for the operating system to make old
    // ports available again
    const int test_ports[test_ports_len] = {
        43711, 42711, 43719, 43911,
        43741, 53991, 42562, 63123,
        58754, 57319, 49871, 59876
    };
    static int next_test_port_idx = 0;

    const int port = test_ports[next_test_port_idx];

    next_test_port_idx = (next_test_port_idx + 1) % test_ports_len;

    udp_socket_init_on_port(sink_socket, port);
    msg_builder_init(builder);

    MemBlock receive_block = mem_block_alloc(1024);

    *source = atolla_source_make("127.0.0.1", port, frame_ms, buffered_frame_count);

    AtollaSourceState source_state = atolla_source_state(*source);
    assert_int_equal(ATOLLA_SOURCE_STATE_WAITING, source_state);
    time_sleep(loopback_send_time_ms);

    UdpSocketResult res = udp_socket_receive(sink_socket, receive_block.data, receive_block.capacity, &receive_block.size, true);

    // a borrow package should have been received by now
    assert_int_equal(res.code, UDP_SOCKET_OK);
    assert_int_not_equal(receive_block.size, 0);

    // verify that a borrow packet with appropriate data was received
    MsgIter iter = msg_iter_make(receive_block.data, receive_block.size);
    assert_int_equal(MSG_TYPE_BORROW, msg_iter_type(&iter));
    assert_int_equal(frame_ms, msg_iter_borrow_frame_length(&iter));
    assert_int_equal(buffered_frame_count, msg_iter_borrow_buffer_length(&iter));

    // send back a lent and wait
    MemBlock* msg = msg_builder_lent(builder);

    res = udp_socket_send(sink_socket, msg->data, msg->size);
    assert_int_equal(res.code, UDP_SOCKET_OK);
    time_sleep(loopback_send_time_ms);

    // now, the connection should be open
    source_state = atolla_source_state(*source);
    assert_int_equal(ATOLLA_SOURCE_STATE_OPEN, source_state);
}

static void test_error_transition(void **state)
{
    AtollaSource source;
    UdpSocket sink_socket;
    MsgBuilder builder;

    setup_test(&source, &sink_socket, &builder);

    // let the sink communicate an unrecoverable error and wait
    MemBlock* msg = msg_builder_fail(&builder, 0, 42);
    udp_socket_send(&sink_socket, msg->data, msg->size);
    time_sleep(loopback_send_time_ms);

    // now, the source should be in error state
    AtollaSourceState source_state = atolla_source_state(source);
    assert_int_equal(ATOLLA_SOURCE_STATE_ERROR, source_state);
}

/**
 * Tests if the source accurately limits framerate by blocking if
 * putting too many frames.
 */
static void test_blocking(void **state)
{
    AtollaSource source;
    UdpSocket sink_socket;
    MsgBuilder builder;

    setup_test(&source, &sink_socket, &builder);

    int start_time = time_now();

    const size_t frame_len = 3;
    uint8_t frame[frame_len] = { 200, 201, 202 };

    for(int i = 0; i < (buffered_frame_count*2); ++i)
    {
        atolla_source_put(source, frame, frame_len);
    }

    int end_time = time_now();

    MemBlock receive_block = mem_block_alloc(1024);

    for(int i = 0; i < 3; ++i)
    {
        UdpSocketResult res = udp_socket_receive(&sink_socket, receive_block.data, receive_block.capacity, &receive_block.size, true);
        assert_int_equal(UDP_SOCKET_OK, res.code);

        MsgIter iter = msg_iter_make(receive_block.data, receive_block.size);
        assert(msg_iter_has_msg(&iter));
        assert_int_equal(MSG_TYPE_ENQUEUE, msg_iter_type(&iter));

        MemBlock sink_frame = msg_iter_enqueue_frame(&iter);
        assert_int_equal(sink_frame.size, frame_len);
        assert_memory_equal(frame, sink_frame.data, frame_len);   
    }

    int duration = end_time - start_time;
    // 20ms tolerance? Ok, I guess, some time necessary to enqueue the first buffered_frame_count
    // frames and for the additional receiving for the simulated sink
    const int tolerance = 20;

    const int expected_duration = buffered_frame_count * frame_ms;
    assert_in_range(duration, expected_duration-tolerance, expected_duration+tolerance);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_error_transition),
        cmocka_unit_test(test_blocking)
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
