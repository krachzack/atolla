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

/**
 * Represents an endpoint for atolla sources to connect to.
 */
struct AtollaSink
{
    void* internal;
};
typedef struct AtollaSink AtollaSink;

/**
 * Initialization parameters for a new sink.
 */
struct AtollaSinkSpec
{
    /**
     * UDP port that the sink will run on.
     */
    int port;
    /**
     * Maximum amount of color-triplets that will be returned from atolla_sink_get.
     * The atolla_sink_get frame buffer must be three times this number in bytes.
     */
    int lights_count;
};
typedef struct AtollaSinkSpec AtollaSinkSpec;

/**
 * Intializes and creates a new sink.
 */
AtollaSink atolla_sink_make(const AtollaSinkSpec* spec);

/**
 * Frees data and resources associated with the sink. The sink
 * cannot be used anymore, unless it is re-initialized with an additional
 * call to atolla_sink_make.
 */
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
