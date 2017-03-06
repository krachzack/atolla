#include <atolla/sink.h>
#include <atolla/source.h>
#include "atolla/config.h"

#if defined(HAVE_POSIX_SLEEP)
    #include <unistd.h>
    #define sleep_ms(ms) (usleep((ms) * 1000))
#elif defined(HAVE_WINDOWS_SLEEP)
    #include <windows.h>
    #define sleep_ms(ms) (Sleep((ms)))
#else
    #error "No sleep function available"
#endif

#include <unistd.h> // for fork
#include <stdio.h>

const int port = 4242;
const int frame_length_ms = 17;
const int max_buffered_frames = 50;

static void run_sink();
static void run_source();

int main(int argc, char* argv[])
{
    pid_t pid = fork();

    if (pid == 0)
    {
        // child process
        run_source();
    }
    else if (pid > 0)
    {
        // parent process
        run_sink();
    }
    else
    {
        // fork failed
        printf("fork() failed!\n");
        return 1;
    }
}

static void run_sink()
{
    printf("Starting atolla sink\n");
    const size_t lights_count = 1;

    AtollaSinkSpec spec;
    spec.port = port;
    spec.lights_count = lights_count;

    AtollaSink sink = atolla_sink_make(&spec);

    while(atolla_sink_state(sink) != ATOLLA_SINK_STATE_LENT)
    {
        sleep_ms(1);
    }
    printf("Sink received borrow\n");

    for(int i = 0; i < max_buffered_frames * 5; ++i)
    {
        uint8_t frame[lights_count*3] = { 1, 2, 3 };
        
        bool has_frame = atolla_sink_get(sink, frame, sizeof(frame) / sizeof(uint8_t));
        
        if(has_frame)
        {
            int red = frame[0];
            int green = frame[1];
            int blue = frame[2];
            
            printf("(%d/%d/%d)\n", red, green, blue);
            sleep_ms(frame_length_ms);
        }
    }
}

static void run_source()
{
    AtollaSourceSpec spec;
    spec.sink_hostname = "127.0.0.1";
    spec.sink_port = port;
    spec.frame_duration_ms = frame_length_ms;
    spec.max_buffered_frames = max_buffered_frames;
    spec.retry_timeout_ms = 0; // 0 means pick a default value
    spec.disconnect_timeout_ms = 0; // 0 means pick a default value

    printf("Starting atolla source\n");

    AtollaSource source = atolla_source_make(&spec);

    while(atolla_source_state(source) != ATOLLA_SOURCE_STATE_OPEN)
    {
        sleep_ms(1);
    }
    printf("Atolla source received lent\n");

    uint8_t frame[] = { 255, 0, 0 };

    for(int i = 0; i < max_buffered_frames * 5; ++i)
    {
        atolla_source_put(source, frame, sizeof(frame) / sizeof(uint8_t));
        sleep_ms(1);
    }
}
