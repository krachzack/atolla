#ifndef ATOLLA_SOURCE_H
#define ATOLLA_SOURCE_H

#include "atolla/primitives.h"

struct AtollaSource
{
    void* recv_buf;
    size_t recv_buf_len;
    void* send_buf;
    size_t send_buf_len;
}
typedef struct AtollaSource AtollaSource;

enum AtollaSourceState
{
    // Channel is in a state of error that it cannot recover from
    ATOLLA_SOURCE_STATE_ERROR,
    // Channel is waiting for a response from the other end or the device was
    // relent
    ATOLLA_SOURCE_STATE_WAITING,
    // Channel is open and ready for reading and writing
    ATOLLA_SOURCE_STATE_OPEN
}
typedef enum AtollaSourceState AtollaSourceState;

AtollaSource atolla_source_make(const char* sink_hostname, int sink_port);

AtollaSourceState atolla_source_state(AtollaSource* source);

/**
 * Tries to enqueue the given frame in the connected sink.
 *
 * The call might block for some time in order to wait for the device to catch
 * up displaying the previously sent frames.
 */
bool atolla_source_put(AtollaSource* source, void* frame, size_t frame_len);

#endif // ATOLLA_SOURCE_H
