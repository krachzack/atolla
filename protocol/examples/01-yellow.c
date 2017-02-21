#include <atolla/sink.h>
#include <atolla/source.h>
#include <atolla/sleep_ms.h>

#include <unistd.h> // for fork
#include <stdio.h>

const int port = 4242;

static void run_sink();
static void run_source();

int main(int argc, char* argv[])
{
    pid_t pid = fork();

    if (pid == 0)
    {
        // child process
        printf("Starting atolla source\n");
        run_source();
    }
    else if (pid > 0)
    {
        // parent process
        printf("Starting atolla sink\n");
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
    const size_t lights_count = 1;
    AtollaSink sink = atolla_sink_make(port, lights_count);
}

static void run_source()
{
    sleep_ms(5); // Give sink some time to initialize

    AtollaSource source = atolla_source_make("127.0.0.1", port);
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