#include "atolla/source.h"
#include "msg/builder.h"
#include "msg/iter.h"
#include "udp_socket/udp_socket.h"
#include "time/sleep.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

/**
 * A time in milliseconds to wait in order to wait for data to be sent through local
 * loopback and arrive at the other local end.
 */
const int loopback_send_time_ms = 5;

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
    const int frame_ms = 17;
    const int buffered_frame_count = 60;
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

    // Verify that a borrow packet with appropriate data was received
    MsgIter iter = msg_iter_make(receive_block.data, receive_block.size);
    assert_int_equal(MSG_TYPE_BORROW, msg_iter_type(&iter));
    assert_int_equal(frame_ms, msg_iter_borrow_frame_length(&iter));
    assert_int_equal(buffered_frame_count, msg_iter_borrow_buffer_length(&iter));

    // Send back a lent and wait
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

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_error_transition),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
