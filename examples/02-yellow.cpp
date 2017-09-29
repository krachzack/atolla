#include <atolla/source.h>
#include <stdio.h>
#include <stdlib.h>

const int frame_length_ms = 17;
const int max_buffered_frames = 50;
const int run_time_ms = 5000;

static void run_source(const char* sink_hostname, int port);
static void paint(AtollaSource source);

static void run_source(const char* sink_hostname, int port)
{
    AtollaSourceSpec spec;
    spec.sink_hostname = sink_hostname;
    spec.sink_port = port;
    spec.frame_duration_ms = frame_length_ms;
    spec.max_buffered_frames = max_buffered_frames;
    spec.retry_timeout_ms = 0; // 0 means pick a default value
    spec.disconnect_timeout_ms = 0; // 0 means pick a default value
    spec.async_make = false;

    printf("Starting atolla source\n");

    AtollaSource source = atolla_source_make(&spec);
    AtollaSourceState state = atolla_source_state(source);

    if(state == ATOLLA_SOURCE_STATE_OPEN) {
        printf("Atolla source received lent\n");
        paint(source);
    } else {
        printf("Error occurred lending\n");
    }
}

static void paint(AtollaSource source)
{
    uint8_t frame[] = { 255, 255, 0 };

    for(int i = 0; (i*frame_length_ms) < run_time_ms; ++i)
    {
        atolla_source_put(source, frame, sizeof(frame) / sizeof(uint8_t));
    }
}

int main(int argc, const char* argv[])
{
    if(argc < 3) {
        run_source("localhost", 10042);
    } else {
        const char* sink_hostname = argv[1];
        int port = atoi(argv[2]);
        run_source(sink_hostname, port);
    }

    return EXIT_SUCCESS;
}
