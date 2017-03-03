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
const int max_buffered_frames = 10;

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
    AtollaSink sink = atolla_sink_make(port, lights_count);

    for(int i = 0; i < 50; ++i)
    {
        uint8_t frame[lights_count*3] = { 0, 0, 0 };
        
        bool has_frame = atolla_sink_get(sink, frame, sizeof(frame) / sizeof(uint8_t));
        
        int red = frame[0];
        int green = frame[1];
        int blue = frame[2];

        printf("(%d/%d/%d)\n", red, green, blue);
        sleep_ms(frame_length_ms);
    }
}

static void run_source()
{
    sleep_ms(5); // Give sink some time to initialize
    printf("Starting atolla source\n");
    AtollaSource source = atolla_source_make("127.0.0.1", port, frame_length_ms, max_buffered_frames);
    uint8_t frame[] = { 255, 0, 0 };

    for(int i = 0; i < 50; ++i)
    {
        atolla_source_put(source, frame, sizeof(frame) / sizeof(uint8_t));
    }
}

/*#include <atolla/client.h>
#include <atolla/sleep_ms.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static const char* ATOLLA_CLIENT_HOSTNAME = "atolla.local";

AtollaClient* borrow();
void check_connection(AtollaClient* client);
void make_yellow(AtollaClient* client);

int main(int argc, char* argv[])
{
    AtollaClient* client = borrow();
    make_yellow(client);
    sleep_ms(5 * 3600 * 1000);
    atolla_client_free(client);
}

AtollaClient* borrow()
{
    AtollaClientBorrow borrow = atolla_client_borrow(
        0, 0 // show sent frames immediately when received
        ATOLLA_CLIENT_HOSTNAME, // hostname or IP of the device to connect to
        50123, // port of remote client
        500, // timeout in milliseconds
        5    // maximum connection attempts before giving up
    );

    // Blocks until client connected or connection failed,
    // e.g. can fail because timed out
    // Use atolla_client_borrow_is_determinate to check for completion in a
    // non-blocking way
    if(atolla_client_borrow_failed(&res))
    {
        AtollaClientBorrowResultCode code = atolla_client_borrow_result_code(&borrow);
        const char* code_str = atolla_client_borrow_result_code_str(&borrow);
        const char* err_msg  = atolla_client_borrow_result_error_msg(&borrow);
        printf("Client borrow failed: %s\n%s\n", code_str, msg);
        exit(1);
        return NULL;
    }
    else
    {
        return atolla_client_borrow_obtain(&borrow);
    }
}

void make_yellow(AtollaClient* client)
{
    uint8_t frame[] = { 255, 255, 0 };
    size_t frame_len = sizeof(frame) / sizeof(uint8_t);

    atolla_client_enqueue(client, frame, frame_len);
    atolla_client_flush(client);
}
*/