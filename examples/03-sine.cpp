#include <atolla/source.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

const int frame_length_ms = 17;
const int max_buffered_frames = 50;
const int run_time_ms = 10000;

static void run_source(const char* sink_hostname, int port);
static void paint(AtollaSource source);

static void paint(AtollaSource source)
{
    static double t = 0.0;
    uint8_t frame[] = { 1, 2, 3 };

    for(int i = 0; (i*frame_length_ms) < run_time_ms; ++i)
    {
        t += frame_length_ms / 1000.0;
        double poscos = (cos(t) + 1.0) / 2.0;
        uint8_t col = (uint8_t) (poscos * 255.0);
        frame[0] = col;
        frame[1] = col;
        frame[2] = col;

        atolla_source_put(source, frame, sizeof(frame) / sizeof(uint8_t));
    }
}

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

    while((state = atolla_source_state(source)) == ATOLLA_SOURCE_STATE_WAITING) {}

    if(state == ATOLLA_SOURCE_STATE_OPEN) {
        printf("Atolla source received lent\n");
        paint(source);
    } else {
        printf("Error occurred lending\n");
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
