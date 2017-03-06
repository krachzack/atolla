#include "atolla/client.h"
#include "atolla/definitions.h"
#include "udp_socket/udp_socket.h"
#include "client_error_messages.h"

#include <stdlib.h>
#include "test/assert.h"

struct AtollaClient
{
    UdpSocket socket;

    AtollaClientState state;

    void* msg_buffer;
    size_t msg_buffer_capacity;
    size_t msg_buffer_len;

    uint8_t frame_length_ms;
    uint8_t frame_buffer_length;

    int timeout_ms;
};

static void borrow_send(AtollaClientBorrow* borrow);
static void borrow_await_completion(AtollaClientBorrow* borrow);

AtollaClientBorrow atolla_client_borrow(
    AtollaClient* client,
    uint8_t frame_length_ms,
    uint8_t frame_buffer_length,
    const char* remote_hostname,
    unsigned short remote_port,
    int timeout_ms,
    int max_connection_attempts
)
{
    UdpSocketResult result = udp_socket_init(&client->socket);
    assert(result.code >= 0);

    client->state = ATOLLA_CLIENT_STATE_BORROWING;

    client->msg_buffer = NULL;
    client->msg_buffer_capacity = 0;
    client->msg_buffer_len = 0;

    client->frame_length_ms = frame_length_ms;
    client->frame_buffer_length = frame_buffer_length;

    AtollaClientBorrow borrow;
    borrow.client = client;
    borrow.code = ATOLLA_CLIENT_BORROW_RESULT_INDETERMINATE;
    borrow.max_connection_attempts = max_connection_attempts;

    result = udp_socket_set_receiver(&client->socket, remote_hostname, remote_port);
    if(result.code == UDP_SOCKET_OK)
    {
        borrow_send(&borrow);
    }
    else if(result.code == UDP_SOCKET_ERR_RESOLVE_HOSTNAME_FAILED)
    {
        borrow.code = ATOLLA_CLIENT_BORROW_RESULT_ERR_RESOLVE_HOSTNAME_FAILED;
    }
    else
    {
        borrow.code = ATOLLA_CLIENT_BORROW_RESULT_ERR_OTHER;
    }

    return borrow;
}

static void borrow_send(AtollaClientBorrow* borrow)
{

}

static void borrow_await_completion(AtollaClientBorrow* borrow)
{
    if(borrow->code == ATOLLA_CLIENT_BORROW_RESULT_INDETERMINATE)
    {
        assert(false);
    }
}

bool atolla_client_borrow_is_determinate(
    AtollaClientBorrow* borrow
)
{
    return borrow->code != ATOLLA_CLIENT_BORROW_RESULT_INDETERMINATE;
}

bool atolla_client_borrow_is_indeterminate(
    AtollaClientBorrow* borrow
)
{
    return borrow->code == ATOLLA_CLIENT_BORROW_RESULT_INDETERMINATE;
}

bool atolla_client_borrow_ok(
    AtollaClientBorrow* borrow
)
{
    borrow_await_completion(borrow);
    return borrow->code == ATOLLA_CLIENT_BORROW_RESULT_OK;
}

bool atolla_client_borrow_failed(
    AtollaClientBorrow* borrow
)
{
    borrow_await_completion(borrow);
    return borrow->code != ATOLLA_CLIENT_BORROW_RESULT_OK;
}

AtollaClientBorrowResultCode atolla_client_borrow_result_code(
    AtollaClientBorrow* borrow
)
{
    return borrow->code;
}

const char* atolla_client_borrow_result_code_str(
    AtollaClientBorrow* borrow
)
{
    switch(borrow->code)
    {
        case ATOLLA_CLIENT_BORROW_RESULT_OK:
            return "ATOLLA_CLIENT_BORROW_RESULT_OK";
        case ATOLLA_CLIENT_BORROW_RESULT_INDETERMINATE:
            return "ATOLLA_CLIENT_BORROW_RESULT_INDETERMINATE";
        case ATOLLA_CLIENT_BORROW_RESULT_ERR_TIMED_OUT:
            return "ATOLLA_CLIENT_BORROW_RESULT_ERR_TIMED_OUT";
        case ATOLLA_CLIENT_BORROW_RESULT_ERR_RESOLVE_HOSTNAME_FAILED:
            return "ATOLLA_CLIENT_BORROW_RESULT_ERR_RESOLVE_HOSTNAME_FAILED";
        case ATOLLA_CLIENT_BORROW_RESULT_ERR_OTHER:
            return "ATOLLA_CLIENT_BORROW_RESULT_ERR_OTHER";
        default:
            assert(false);
            return NULL;
    }
}

const char* atolla_client_borrow_result_error_msg(
    AtollaClientBorrow* borrow
)
{
    switch(borrow->code)
    {
        case ATOLLA_CLIENT_BORROW_RESULT_OK:
            return NULL;
        case ATOLLA_CLIENT_BORROW_RESULT_INDETERMINATE:
            return ATOLLA_CLIENT_BORROW_RESULT_INDETERMINATE_MSG;
        case ATOLLA_CLIENT_BORROW_RESULT_ERR_TIMED_OUT:
            return ATOLLA_CLIENT_BORROW_RESULT_ERR_TIMED_OUT_MSG;
            break;
        case ATOLLA_CLIENT_BORROW_RESULT_ERR_RESOLVE_HOSTNAME_FAILED:
            return ATOLLA_CLIENT_BORROW_RESULT_ERR_RESOLVE_HOSTNAME_FAILED_MSG;
            break;
        case ATOLLA_CLIENT_BORROW_RESULT_ERR_OTHER:
            return ATOLLA_CLIENT_BORROW_RESULT_ERR_OTHER_MSG;
            break;
        default:
            assert(false);
            return NULL;
    }
}

AtollaClient* atolla_client_borrow_obtain(
    AtollaClientBorrow* borrow
)
{
    borrow_await_completion(borrow);
    return borrow->client;
}

AtollaClientState atolla_client_state(
    AtollaClient* client
)
{
    return client->state;
}

/**
 * Enqueue a frame into an internal buffer to later send it to the client using
 * the atolla_client_flush function.
 */
void atolla_client_enqueue(
    AtollaClient* client,
    uint8_t* frame,
    size_t frame_length
)
{
    assert(false);
}

/**
 * Sends all pending enqueued frames to the client.
 */
void atolla_client_flush(
    AtollaClient* client
)
{
    assert(false);
}

void atolla_client_free(
    AtollaClient* client
)
{
    assert(false);
}