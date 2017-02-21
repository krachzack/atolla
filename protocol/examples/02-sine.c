/*#include <atolla/client.h>
#include <atolla/sleep_ms.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

const char* ATOLLA_CLIENT_HOSTNAME = "atolla.local";

AtollaClient* borrow();
bool check_connection(AtollaClient** client);
void show_sine(AtollaClient* client);*/

int main(int argc, char* argv[])
{
    /*AtollaClient* client = borrow();

    while(client && check_connection(&client)) {
        show_sine(client);
        sleep_ms(17);
    }

    atolla_client_free(client);*/
}


/*AtollaClient* borrow()
{
    AtollaClientBorrow borrow = atolla_client_borrow(
        17, // frame length in ms, equivavalent to ~60 frames per second
        30, // buffer thirty frames into the future for lag compensation
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
        return NULL;
    }
    else
    {
        return atolla_client_borrow_obtain(&borrow);
    }
}


bool check_connection(AtollaClient** client)
{
    bool connected = true;
    AtollaClientState state = atolla_client_state(*client);

    if(state != ATOLLA_CLIENT_STATE_OK)
    {
        // The device either did not send messages back for some time or it has
        // been borrowed by someone else
        if(state == ATOLLA_CLIENT_STATE_UNRESPONSIVE)
        {
            printf("The device did not phone home in a while, trying to reconnect...\n");
            atolla_client_free(*client);
            *client = borrow();
            connected = *client != NULL;
        }
        else if(state == ATOLLA_CLIENT_STATE_RELENT)
        {
            printf("The device has been lent to another client, exiting...\n")
            connected = false;
        }
        else
        {
            printf(
                "Device has error state: %s\n",
                atolla_client_state_str(*client);
            );
            connected = false;
        }
    }

    return connected;
}

void show_sine(AtollaClient* client)
{
    // alternatively use atolla_client_next_frame_delta to see how much time
    // into the future the next frame is instead of getting an absolute time
    // for the frame
    for(long next_frame_time = atolla_client_next_frame_time(client);
        next_frame_time != -1;
        next_frame_time = atolla_client_next_frame_time(client))
    {
        double next_frame_time_secs = atolla_client_next_frame_time / 1000.0;
        double sine_normalized = (sin(next_frame_time_secs) + 1.0) / 2.0;
        uint8_t sine_int = sine_normalized * 255;
        uint8_t frame[] = { sine_int, sine_int, sine_int };
        size_t frame_length = sizeof(frame) / sizeof(uint8_t);

        atolla_client_enqueue(client, frame, frame_length);
    }

    atolla_client_flush(client);
}*/
