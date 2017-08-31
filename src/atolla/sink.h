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
    void* internal;
};
typedef struct AtollaSink AtollaSink;

struct AtollaSinkSpec
{
    int port;
    int lights_count;
};
typedef struct AtollaSinkSpec AtollaSinkSpec;

AtollaSink atolla_sink_make(const AtollaSinkSpec* spec);

void atolla_sink_free(AtollaSink sink);

/**
 * Redetermines the state of the sink based on incoming packets.
 */
AtollaSinkState atolla_sink_state(AtollaSink sink);

/**
 * Gets the current frame based on the instant in time upon calling the
 * function.
 *
 * Note that atolla_sink_state must be regularly called to evaluate
 * incoming packets in order for atolla_sink_get to provide results.
 *
 * If returns false, no frame available yet.
 */
bool atolla_sink_get(AtollaSink sink, void* frame, size_t frame_len);

#endif // ATOLLA_SINK_H
