#ifndef ATOLLA_SOURCE_H
#define ATOLLA_SOURCE_H

#include "atolla/primitives.h"

enum AtollaSourceState
{
    // Channel is in a state of error that it cannot recover from
    ATOLLA_SOURCE_STATE_ERROR,
    // Channel is waiting for a response from the other end or the device was
    // relent
    ATOLLA_SOURCE_STATE_WAITING,
    // Communcation is ongoing
    ATOLLA_SOURCE_STATE_OPEN
};
typedef enum AtollaSourceState AtollaSourceState;

struct AtollaSource
{
    void* private;
};
typedef struct AtollaSource AtollaSource;

AtollaSource atolla_source_make(const char* sink_hostname, int sink_port, int frame_length_ms, int max_buffered_frames);

/**
 * Orderly shuts down the source first and then frees associated resources.
 * The source referenced by the given source handle may not be used after calling
 * this function.
 */
void atolla_source_free(AtollaSource source);

AtollaSourceState atolla_source_state(AtollaSource source);

/**
 * Determines how many frames have to be sent to the sink to fill its buffer
 * at the instant of calling the function.
 *
 * If this function returns a non-zero value, the atolla_source_put function
 * will never block. In the zero case, the put function will halt until the
 * device is ready to receive the next frame.
 */
int atolla_source_frame_lag(AtollaSource source);

/**
 * Tries to enqueue the given frame in the connected sink.
 *
 * The call might block for some time in order to wait for the device to catch
 * up displaying the previously sent frames. If atolla_source_frame_lag returns
 * a non-zero value, atolla_source_put is guaranteed not to wait.
 */
bool atolla_source_put(AtollaSource source, void* frame, size_t frame_len);

#endif // ATOLLA_SOURCE_H
