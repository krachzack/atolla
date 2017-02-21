#ifndef ATOLLA_SINK_H
#define ATOLLA_SINK_H

#include "atolla/primitives.h"

struct AtollaSink
{
    void* recv_buf;
    size_t recv_buf_len;
    void* send_buf;
    size_t send_buf_len;
}
typedef struct AtollaSink AtollaSink;

enum AtollaSinkState
{
    // Channel is in a state of error that it cannot recover from
    ATOLLA_SINK_STATE_ERROR,
    // Channel is waiting for a response from the other end or the device was
    // relent
    ATOLLA_SINK_STATE_WAITING,
    // Channel is open and ready for reading and writing
    ATOLLA_SINK_STATE_OPEN
}
typedef enum AtollaSinkState AtollaSinkState;

AtollaSink atolla_sink_make(int port);

AtollaSinkState atolla_sink_state(AtollaSink* sink);

bool atolla_sink_get(AtollaSink* sink, int frame_idx, void* frame, size_t frame_len);

#endif // ATOLLA_SINK_H
