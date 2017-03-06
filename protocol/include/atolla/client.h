#ifndef ATOLLA_CLIENT_H
#define ATOLLA_CLIENT_H

#include "atolla/primitives.h"

/**
 * Represents a connection to an atolla device.
 * Call <code>atolla_client_borrow</code> to initialize the structure and start
 * a connection attempt. No other function may be used with an unitialized
 * client.
 */
struct AtollaClient;
typedef struct AtollaClient AtollaClient;

enum AtollaClientState
{
    ATOLLA_CLIENT_STATE_OK = 0,
    ATOLLA_CLIENT_STATE_BORROWING = -1,
    ATOLLA_CLIENT_STATE_BORROW_FAILED = -2,
    ATOLLA_CLIENT_STATE_UNRESPONSIVE = -3,
    ATOLLA_CLIENT_STATE_RELENT = -4
};
typedef enum AtollaClientState AtollaClientState;

enum AtollaClientBorrowResultCode
{
    ATOLLA_CLIENT_BORROW_RESULT_OK = 0,
    ATOLLA_CLIENT_BORROW_RESULT_INDETERMINATE = -1,
    ATOLLA_CLIENT_BORROW_RESULT_ERR_TIMED_OUT = -2,
    ATOLLA_CLIENT_BORROW_RESULT_ERR_RESOLVE_HOSTNAME_FAILED = -3,
    ATOLLA_CLIENT_BORROW_RESULT_ERR_OTHER = -4
};
typedef enum AtollaClientBorrowResultCode AtollaClientBorrowResultCode;

/**
 * Represents a connection attempt to an atolla device. The structure is no
 * longer required after the connection attempt has concluded.
 */
struct AtollaClientBorrow
{
    AtollaClient* client;
    AtollaClientBorrowResultCode code;
    int max_connection_attempts;
};
typedef struct AtollaClientBorrow AtollaClientBorrow;

/**
 *
 */
AtollaClientBorrow atolla_client_borrow(
    AtollaClient* client,
    uint8_t frame_length_ms,
    uint8_t frame_buffer_length,
    const char* remote_hostname,
    unsigned short remote_port,
    int timeout_ms,
    int max_connection_attempts
);

/**
 *
 */
bool atolla_client_borrow_is_determinate(
    AtollaClientBorrow* borrow
);

/**
 *
 */
bool atolla_client_borrow_is_indeterminate(
    AtollaClientBorrow* borrow
);

/**
 *
 */
bool atolla_client_borrow_ok(
    AtollaClientBorrow* borrow
);

/**
 *
 */
bool atolla_client_borrow_failed(
    AtollaClientBorrow* borrow
);

/**
 *
 */
AtollaClientBorrowResultCode atolla_client_borrow_result_code(
    AtollaClientBorrow* borrow
);

/**
 *
 */
const char* atolla_client_borrow_result_code_str(
    AtollaClientBorrow* borrow
);

/**
 *
 */
const char* atolla_client_borrow_result_error_msg(
    AtollaClientBorrow* borrow
);

/**
 * Gets a connected client after a successful borrow process. If borrowing
 * failed, <code>NULL</code> is returned. If the borrow process is still
 * indeterminate, this function blocks until it either failed or succeeded.
 */
AtollaClient* atolla_client_borrow_obtain(
    AtollaClientBorrow* borrow
);

/**
 *
 */
AtollaClientState atolla_client_state(
    AtollaClient* client
);

/**
 * Enqueue a frame into an internal buffer to later send it to the client using
 * the atolla_client_flush function.
 */
void atolla_client_enqueue(
    AtollaClient* client,
    uint8_t* frame,
    size_t frame_length
);

/**
 * Sends all pending enqueued frames to the client.
 */
void atolla_client_flush(
    AtollaClient* client
);

void atolla_client_free(
    AtollaClient* client
)
{
    assert(false);
}

#endif // ATOLLA_CLIENT_H
