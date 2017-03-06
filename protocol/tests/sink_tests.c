#include "atolla/sink.h"
#include "udp_socket/udp_socket.h"
#include "msg/builder.h"
#include "msg/iter.h"
#include "time/sleep.h"
#include "time/now.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

static const int port = 61489;
static const uint8_t frame_length = 17;
static const uint8_t buffered_frame_count = 50;
/**
 * A time in milliseconds to wait in order to wait for data to be sent through local
 * loopback and arrive at the other local end.
 */
static const int loopback_send_time_ms = 5;

static void setup_open_sink(AtollaSink* sink, UdpSocket* source_sock, MsgBuilder* builder)
{
    AtollaSinkSpec spec;
    spec.port = port;
    spec.lights_count = 1;

    *sink = atolla_sink_make(&spec);
    assert_int_equal(ATOLLA_SINK_STATE_OPEN, atolla_sink_state(*sink));

    udp_socket_init(source_sock);
    udp_socket_set_receiver(source_sock, "127.0.0.1", port);

    msg_builder_init(builder);
}

static void setup_lent_sink(AtollaSink* sink, UdpSocket* source_sock, MsgBuilder* builder)
{
    setup_open_sink(sink, source_sock, builder);

    // Borrow the sink with a virtual source represented by the socket
    MemBlock* msg = msg_builder_borrow(builder, frame_length, buffered_frame_count);
    UdpSocketResult res = udp_socket_send(source_sock, msg->data, msg->size);
    assert_int_equal(UDP_SOCKET_OK, res.code);
    time_sleep(loopback_send_time_ms);

    assert_int_equal(ATOLLA_SINK_STATE_LENT, atolla_sink_state(*sink));
}

/**
 * Tests if sending enqueue messages really mutates the sink frame buffer.
 */
static void test_fill_sink_buf(void **state)
{
    AtollaSink sink;
    UdpSocket source_sock;
    MsgBuilder builder;

    setup_lent_sink(&sink, &source_sock, &builder);

    const size_t frame_len = 3;
    uint8_t frame[frame_len] = { 0, 0, 0 };

    const int before_enqueue = time_now();
    for(int i = 0; i < buffered_frame_count; ++i)
    {
        frame[0] = (uint8_t) i;
        frame[1] = (uint8_t) i;
        frame[2] = (uint8_t) i;

        MemBlock* msg = msg_builder_enqueue(&builder, i, frame, frame_len);
        UdpSocketResult res = udp_socket_send(&source_sock, msg->data, msg->size);
        assert_int_equal(UDP_SOCKET_OK, res.code);

        time_sleep(1);
        atolla_sink_state(sink); // this just makes sure the sink gets updates
    }

    for(int i = 0; i < buffered_frame_count/2; ++i)
    {
        const int frames_passed = (time_now() - before_enqueue) / frame_length;
        const int tolerance = 1;
        const int expected_color_min = (frames_passed == 0) ? 0 : (frames_passed - tolerance);
        const int expected_color_max = frames_passed + tolerance;

        atolla_sink_get(sink, frame, frame_len);

        assert_in_range(frame[0], expected_color_min, expected_color_max);
        assert_in_range(frame[1], expected_color_min, expected_color_max);
        assert_in_range(frame[2], expected_color_min, expected_color_max);

        time_sleep(frame_length);
    }
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_fill_sink_buf),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
