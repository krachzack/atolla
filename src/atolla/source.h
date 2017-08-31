#ifndef ATOLLA_SOURCE_H
#define ATOLLA_SOURCE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "atolla/primitives.h"

enum AtollaSourceState
{
    /**
     * The source has sent a message to the sink that communicates the desire
     * of the source to stream light information to the sink, which should
     * then be displayed. While in this state, the source waits for the
     * first response of the sink, ensuring that the sink exists and is
     * functioning.
     */
    ATOLLA_SOURCE_STATE_WAITING,
    /**
     * The sink is lent to the source and light information can be streamed with
     * atolla_source_put.
     */
    ATOLLA_SOURCE_STATE_OPEN,
    /**
     * Channel is in a state of error that it cannot recover from.
     * Possible reasons for this are:
     *
     * - The sink did not respond when trying to establish a connection,
     * - the connection was lost later,
     * - the sink has been lent to another source,
     * - the sink has communicated an unrecoverable error.
     */
    ATOLLA_SOURCE_STATE_ERROR
};
typedef enum AtollaSourceState AtollaSourceState;

/**
 * Represents the source of streaming light information that is connected
 * to a sink.
 */
struct AtollaSource
{
    void* private;
};
typedef struct AtollaSource AtollaSource;

struct AtollaSourceSpec
{
    const char* sink_hostname;
    int sink_port;
    int frame_duration_ms;
    int max_buffered_frames;
    /**
     * Holds the time in milliseconds after which a new borrow message is
     * sent in reaction to a suspected packet loss.
     *
     * A value of zero lets the implementation pick a default value.
     */
    int retry_timeout_ms;
    /**
     * Holds the time in milliseconds after which no further attempts are
     * made to contact the sink. If the sink is unresponsive for this amount
     * of milliseconds, the source enters the error state.
     *
     * A value of zero lets the implementation pick a default value.
     */
    int disconnect_timeout_ms;
};
typedef struct AtollaSourceSpec AtollaSourceSpec;

AtollaSource atolla_source_make(const AtollaSourceSpec* spec);

/**
 * Orderly shuts down the source first and then frees associated resources.
 * The source referenced by the given source handle may not be used again
 * after calling this function unless it is re-initialized with atolla_source_make.
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

#ifdef __cplusplus
}
#endif

#endif // ATOLLA_SOURCE_H
