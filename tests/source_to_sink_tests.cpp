#include "atolla/source.h"
#include "atolla/sink.h"
#include "time/sleep.h"

extern "C" {
    #include <stdarg.h>
    #include <stddef.h>
    #include <setjmp.h>
    #include <cmocka.h>
}

const int port = 10001;
const int frame_duration_ms = 30;
static const int loopback_send_time_ms = 5;

static void test_connect(void **state)
{
    AtollaSourceSpec source_spec;
    source_spec.sink_hostname = "127.0.0.1";
    source_spec.sink_port = port;
    source_spec.frame_duration_ms = frame_duration_ms;
    source_spec.max_buffered_frames = 0;
    source_spec.retry_timeout_ms = 0;
    source_spec.disconnect_timeout_ms = 0;
    source_spec.async_make = true;

    AtollaSinkSpec sink_spec;
    sink_spec.port = port;
    sink_spec.lights_count = 1;

    AtollaSink sink = atolla_sink_make(&sink_spec);
    assert_int_equal(ATOLLA_SINK_STATE_OPEN, atolla_sink_state(sink));

    AtollaSource source = atolla_source_make(&source_spec);
    assert_int_equal(ATOLLA_SOURCE_STATE_WAITING, atolla_source_state(source));
    time_sleep(loopback_send_time_ms);

    atolla_sink_state(sink); // Let sink handle request and wait a bit
    time_sleep(loopback_send_time_ms);

    assert_int_equal(ATOLLA_SINK_STATE_LENT, atolla_sink_state(sink));
    assert_int_equal(ATOLLA_SOURCE_STATE_OPEN, atolla_source_state(source));

    atolla_source_free(source);
    atolla_sink_free(sink);
}

static void test_stream_rising(void **state)
{
    AtollaSourceSpec source_spec;
    source_spec.sink_hostname = "127.0.0.1";
    source_spec.sink_port = port;
    source_spec.frame_duration_ms = frame_duration_ms;
    source_spec.max_buffered_frames = 0;
    source_spec.retry_timeout_ms = 0;
    source_spec.disconnect_timeout_ms = 0;
    source_spec.async_make = true;

    AtollaSinkSpec sink_spec;
    sink_spec.port = port;
    sink_spec.lights_count = 1;

    AtollaSink sink = atolla_sink_make(&sink_spec);
    assert_int_equal(ATOLLA_SINK_STATE_OPEN, atolla_sink_state(sink));

    AtollaSource source = atolla_source_make(&source_spec);
    assert_int_equal(ATOLLA_SOURCE_STATE_WAITING, atolla_source_state(source));
    time_sleep(loopback_send_time_ms);

    atolla_sink_state(sink); // Let sink handle request and wait a bit
    time_sleep(loopback_send_time_ms);

    assert_int_equal(ATOLLA_SINK_STATE_LENT, atolla_sink_state(sink));
    assert_int_equal(ATOLLA_SOURCE_STATE_OPEN, atolla_source_state(source));

    const int buffered_frame_count = atolla_source_put_ready_count(source);
    for(uint8_t i = 0; i < buffered_frame_count; ++i)
    {
        uint8_t frame[3] = { i, i, i }; 
        atolla_source_put(source, frame, 3);
    }

    time_sleep(loopback_send_time_ms);
    int tolerance = 1;

    for(uint8_t i = 0; i < buffered_frame_count; ++i)
    {
        uint8_t frame[3] = { ~i, ~i, ~i };
        atolla_sink_state(sink); // update the sink
        bool ok = atolla_sink_get(sink, frame, 3);

        if(ok) {
            int diff = ((int) i) - ((int) frame[0]);
            if(diff < 0) diff = -diff;
            assert_true(diff <= tolerance);
            time_sleep(frame_duration_ms);
        } else {
            --i;
        }
    }

    atolla_source_free(source);
    atolla_sink_free(sink);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_connect),
        cmocka_unit_test(test_stream_rising)
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
