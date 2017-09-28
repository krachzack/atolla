#include "atolla/sink.h"
#include "udp_socket/udp_socket.h"
#include "msg/builder.h"
#include "msg/iter.h"
#include "time/sleep.h"
#include "time/now.h"

extern "C" {
    #include <stdarg.h>
    #include <stddef.h>
    #include <setjmp.h>
    #include <cmocka.h>
}

static const int port = 61489;
static const uint8_t frame_length = 17;
static const uint8_t buffered_frame_count = 50;
static const int lights_count = 12;

/**
 * A time in milliseconds to wait in order to wait for data to be sent through local
 * loopback and arrive at the other local end.
 */
static const int loopback_send_time_ms = 5;

static void setup_open_sink(AtollaSink* sink, UdpSocket* source_sock, MsgBuilder* builder)
{
    AtollaSinkSpec spec;
    spec.port = port;
    spec.lights_count = lights_count;

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

    time_sleep(loopback_send_time_ms);

    uint8_t buf[256];
    size_t received_bytes;
    res = udp_socket_receive(source_sock, buf, 256, &received_bytes, false);
    assert_int_equal(UDP_SOCKET_OK, res.code);
    assert_true(received_bytes > 3);
    assert_int_equal(1, buf[0]); // Message type byte should be 1 for a LENT message
}

static void teardown_sink(AtollaSink sink, UdpSocket* source_sock, MsgBuilder* builder)
{
    atolla_sink_free(sink);
    udp_socket_free(source_sock);
    msg_builder_free(builder);
}

/**
 * Tests by sending enqueue messages and then dequeuing them.
 */
static void test_fill_sink_buf(void **state)
{
    AtollaSink sink;
    UdpSocket source_sock;
    MsgBuilder builder;

    setup_lent_sink(&sink, &source_sock, &builder);

    const size_t frame_len = 3;
    uint8_t frame[frame_len] = { 0, 0, 0 };

    for(int i = 0; i < buffered_frame_count; ++i)
    {
        frame[0] = (uint8_t) i;
        frame[1] = (uint8_t) i;
        frame[2] = (uint8_t) i;

        MemBlock* msg = msg_builder_enqueue(&builder, i, frame, frame_len);
        UdpSocketResult res = udp_socket_send(&source_sock, msg->data, msg->size);
        if(res.code != UDP_SOCKET_OK)
        {
            --i;
        }
        else
        {
            atolla_sink_state(sink); // this just makes sure the sink gets the updates
        }
    }

    const int origin_time = time_now();
    for(int i = 0; i < buffered_frame_count/2; ++i)
    {
        const int frames_passed = (time_now() - origin_time) / frame_length;
        const int tolerance = 1;
        const int expected_color_min = (frames_passed == 0) ? 0 : (frames_passed - tolerance);
        const int expected_color_max = frames_passed + tolerance;

        atolla_sink_get(sink, frame, frame_len);

        assert_in_range(frame[0], expected_color_min, expected_color_max);
        assert_in_range(frame[1], expected_color_min, expected_color_max);
        assert_in_range(frame[2], expected_color_min, expected_color_max);

        time_sleep(frame_length);
    }

    teardown_sink(sink, &source_sock, &builder);
}

static void test_lend_resend(void **state)
{
    AtollaSink sink;
    UdpSocket source_sock;
    MsgBuilder builder;

    setup_lent_sink(&sink, &source_sock, &builder);

    const int expected_relent_interval_ms = 500;

    const size_t frame_len = 3;
    uint8_t frame[frame_len] = { 0, 0, 0 };


    for(int i = 0; i < 3; ++i)
    {
        // Enqueue one second worth of frames to trigger a relent
        // If we would not enqueue frames, the sink would drop the connection
        for(int i = 0; i < ((expected_relent_interval_ms / frame_length) + 1); ++i)
        {
            frame[0] = (uint8_t) i;
            frame[1] = (uint8_t) i;
            frame[2] = (uint8_t) i;

            MemBlock* msg = msg_builder_enqueue(&builder, i, frame, frame_len);
            UdpSocketResult res = udp_socket_send(&source_sock, msg->data, msg->size);
            if(res.code != UDP_SOCKET_OK)
            {
                --i;
            }
            else
            {
                time_sleep(frame_length);
                atolla_sink_state(sink); // this just makes sure the sink gets the updates
            }
        }

        time_sleep(loopback_send_time_ms);
        uint8_t buf[256];
        size_t received_bytes;
        UdpSocketResult res = udp_socket_receive(&source_sock, buf, 256, &received_bytes, false);
        assert_int_equal(UDP_SOCKET_OK, res.code);
        assert_true(received_bytes > 3);
        assert_int_equal(1, buf[0]); // Message type byte should be 1 for a LENT message
    }

    teardown_sink(sink, &source_sock, &builder);
}

static void test_get_repeat_pattern(void **state)
{
    AtollaSink sink;
    UdpSocket source_sock;
    MsgBuilder builder;

    setup_lent_sink(&sink, &source_sock, &builder);

    const size_t frame_len = 6;
    uint8_t frame[frame_len] = { 0, 0, 0, 255, 255, 255 };

    for(int i = 0; i < buffered_frame_count; ++i)
    {
        MemBlock* msg = msg_builder_enqueue(&builder, i, frame, frame_len);
        UdpSocketResult res = udp_socket_send(&source_sock, msg->data, msg->size);
        if(res.code != UDP_SOCKET_OK)
        {
            --i;
        }
        else
        {
            atolla_sink_state(sink); // this just makes sure the sink gets the updates
        }
    }

    uint8_t got_frame[lights_count*3];
    atolla_sink_get(sink, got_frame, lights_count*3);
    assert_memory_equal(got_frame, frame, frame_len);
    assert_memory_equal(got_frame+frame_len, frame, frame_len);
    assert_memory_equal(got_frame+frame_len+frame_len, frame, frame_len);

    teardown_sink(sink, &source_sock, &builder);
}

static void test_error_if_port_in_use(void **state)
{
    AtollaSinkSpec spec;
    spec.port = 11110;
    spec.lights_count = lights_count;

    AtollaSink sink1 = atolla_sink_make(&spec);
    assert_int_equal(ATOLLA_SINK_STATE_OPEN, atolla_sink_state(sink1));
    assert_ptr_equal(NULL, atolla_sink_error_msg(sink1));

    AtollaSink sink2 = atolla_sink_make(&spec);
    // Trying to open sink on same port should fail
    assert_int_equal(ATOLLA_SINK_STATE_ERROR, atolla_sink_state(sink2));
    assert_ptr_not_equal(NULL, atolla_sink_error_msg(sink2));

    // But the old sink should remain intact
    assert_int_equal(ATOLLA_SINK_STATE_OPEN, atolla_sink_state(sink1));
    assert_ptr_equal(NULL, atolla_sink_error_msg(sink1));

    atolla_sink_free(sink1);
    atolla_sink_free(sink2);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_fill_sink_buf),
        cmocka_unit_test(test_lend_resend),
        cmocka_unit_test(test_get_repeat_pattern),
        cmocka_unit_test(test_error_if_port_in_use)
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
