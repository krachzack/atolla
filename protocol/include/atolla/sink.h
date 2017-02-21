#ifndef ATOLLA_SINK_H
#define ATOLLA_SINK_H

#include "atolla/primitives.h"

enum AtollaSinkState
{
    // Sink is in a state of error that it cannot recover from
    ATOLLA_SINK_STATE_ERROR,
    // Sink is ready for a source to connect to it
    ATOLLA_SINK_STATE_OPEN,
    // Sink is currently connected to a source
    ATOLLA_SINK_STATE_LENT
};
typedef enum AtollaSinkState AtollaSinkState;

struct AtollaSink
{
    AtollaSinkState state;
    int lights_count;
    int socket_handle; // Contains socket ID to hide UdpSocket type
    void* recv_buf;
    int frame_duration_ms;
    void* frame_buf;
    size_t frame_buf_len;
};
typedef struct AtollaSink AtollaSink;

AtollaSink atolla_sink_make(int udp_port, int lights_count);

AtollaSinkState atolla_sink_state(AtollaSink* sink);

void atolla_sink_update(AtollaSink* sink);

bool atolla_sink_get(AtollaSink* sink, void* frame, size_t frame_len);

#endif // ATOLLA_SINK_H
