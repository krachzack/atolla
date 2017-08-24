#include "atolla/source.h"
#include "msg/builder.h"
#include "msg/iter.h"
#include "test/assert.h"
#include "time/now.h"
#include "time/sleep.h"
#include "udp_socket/udp_socket.h"

#include <stdlib.h>

static const size_t recv_buf_len = 3 + 65537;
static const int retry_timeout_ms_default = 10;
static const int disconnect_timeout_ms_default = retry_timeout_ms_default * 10;
static const int max_buffered_frames_default = 16;

struct AtollaSourcePrivate
{
    AtollaSourceState state;
    UdpSocket sock;
    void* recv_buf;
    
    MsgBuilder builder;

    int next_frame_idx;
    int frame_length_ms;
    int max_buffered_frames;
    int retry_timeout_ms;
    int disconnect_timeout_ms;

    int first_borrow_time;
    int last_borrow_time;
    int last_frame_time;
};
typedef struct AtollaSourcePrivate AtollaSourcePrivate;

static AtollaSourcePrivate* source_private_make(const AtollaSourceSpec* spec);
static void source_send_borrow(AtollaSourcePrivate* source);
static void source_update(AtollaSourcePrivate* source);
static void source_iterate_recv_buf(AtollaSourcePrivate* sink, size_t received_bytes);
static void source_lent(AtollaSourcePrivate* source);
static void source_fail(AtollaSourcePrivate* source);
static void source_receive(AtollaSourcePrivate* source);
static void source_manage_borrow_packet_loss(AtollaSourcePrivate* source);

AtollaSource atolla_source_make(const AtollaSourceSpec* spec)
{
    assert(spec->sink_port >= 0 && spec->sink_port < 65536);

    AtollaSourcePrivate* source = source_private_make(spec);

    msg_builder_init(&source->builder);

    UdpSocketResult result;
    
    result = udp_socket_init(&source->sock);
    assert(result.code == UDP_SOCKET_OK);

    result = udp_socket_set_receiver(&source->sock, spec->sink_hostname, (unsigned short) spec->sink_port);
    if(result.code == UDP_SOCKET_OK) {
        // If hostname could be resolved, send first borrow
        source->first_borrow_time = time_now();
        source_send_borrow(source);
    } else {
        // If resolving failed, immediately enter error state
        source->state = ATOLLA_SOURCE_STATE_ERROR;
    }

    AtollaSource source_handle = { source };
    return source_handle;
}

static AtollaSourcePrivate* source_private_make(const AtollaSourceSpec* spec)
{
    AtollaSourcePrivate* source = (AtollaSourcePrivate*) malloc(sizeof(AtollaSourcePrivate));

    source->state = ATOLLA_SOURCE_STATE_WAITING;
    source->recv_buf = malloc(recv_buf_len);
    source->next_frame_idx = 0;
    source->frame_length_ms = spec->frame_duration_ms;
    source->max_buffered_frames = (spec->max_buffered_frames == 0) ? max_buffered_frames_default : spec->max_buffered_frames;
    source->retry_timeout_ms = (spec->retry_timeout_ms == 0) ? retry_timeout_ms_default : spec->retry_timeout_ms;
    source->disconnect_timeout_ms = (spec->disconnect_timeout_ms == 0) ? disconnect_timeout_ms_default : spec->disconnect_timeout_ms;

    source->first_borrow_time = 0;
    source->last_borrow_time = 0;
    source->last_frame_time = 0;

    return source;
}

void atolla_source_free(AtollaSource source_handle)
{
    AtollaSourcePrivate* source = (AtollaSourcePrivate*) source_handle.private;

    UdpSocketResult result = udp_socket_free(&source->sock);
    assert(result.code == UDP_SOCKET_OK);

    free(source);
}

AtollaSourceState atolla_source_state(AtollaSource source_handle)
{
    AtollaSourcePrivate* source = (AtollaSourcePrivate*) source_handle.private;

    source_update(source);

    return source->state;
}

int atolla_source_frame_lag(AtollaSource source_handle)
{
    AtollaSourcePrivate* source = (AtollaSourcePrivate*) source_handle.private;

    source_update(source);

    if(source->state == ATOLLA_SOURCE_STATE_ERROR)
    {
        // If in unrecoverable error state, report lagging behind 0 frames
        return 0;
    }
    else if(source->state == ATOLLA_SOURCE_STATE_WAITING)
    {
        // If not fully connected yet, report maximum lag
        return source->max_buffered_frames;
    }
    else
    {
        assert(source->state == ATOLLA_SOURCE_STATE_OPEN);

        if(source->last_frame_time == -1)
        {
            // If connected, but no frame was enqueued yet, also report maximum lag
            return source->max_buffered_frames;
        }
        else
        {
            // Otherwise, calculate lag based on the time of the last enqueued frame
            return (time_now() - source->last_frame_time) / source->frame_length_ms;
        }
    }
}

/**
 * Tries to enqueue the given frame in the connected sink.
 *
 * The call might block for some time in order to wait for the device to catch
 * up displaying the previously sent frames.
 */
bool atolla_source_put(AtollaSource source_handle, void* frame, size_t frame_len)
{
    AtollaSourcePrivate* source = (AtollaSourcePrivate*) source_handle.private;

    source_update(source);

    if(atolla_source_state(source_handle) != ATOLLA_SOURCE_STATE_OPEN)
    {
        return false;
    }

    int now = time_now();
    int next_frame_time = source->last_frame_time + source->frame_length_ms;
    if(next_frame_time > now)
    {
        // If the receiving device has no space in the buffer to hold new frames,
        // wait until the next frame was dequeued in the sink
        time_sleep(next_frame_time - now);
    }

    MemBlock* enqueue_msg = msg_builder_enqueue(&source->builder, source->next_frame_idx, frame, frame_len);
    udp_socket_send(&source->sock, enqueue_msg->data, enqueue_msg->size);

    source->next_frame_idx = (source->next_frame_idx + 1) % 256;

    if(source->last_frame_time == -1)
    {
        source->last_frame_time = time_now() - (source->max_buffered_frames - 1) * source->frame_length_ms;
    }
    else
    {
        // Otherwise, advanace the last frame time, so we get closer to the point where no more
        // frame can be enqueued
        source->last_frame_time += source->frame_length_ms;
    }

    return true;
}

static void source_send_borrow(AtollaSourcePrivate* source)
{
    source->last_borrow_time = time_now();
    MemBlock* borrow_msg = msg_builder_borrow(&source->builder, source->frame_length_ms, source->max_buffered_frames);
    udp_socket_send(&source->sock, borrow_msg->data, borrow_msg->size);
}

static void source_update(AtollaSourcePrivate* source)
{
    source_receive(source);
    source_manage_borrow_packet_loss(source);
}

static void source_receive(AtollaSourcePrivate* source)
{
    size_t received_len;
    UdpSocketResult result;

    result = udp_socket_receive(
        &source->sock,
        source->recv_buf, recv_buf_len,
        &received_len,
        false
    );

    if(result.code == UDP_SOCKET_OK)
    {
        source_iterate_recv_buf(source, received_len);
    }
}

static void source_manage_borrow_packet_loss(AtollaSourcePrivate* source)
{
    if(source->state == ATOLLA_SOURCE_STATE_WAITING)
    {
        int now = time_now();
        int time_since_first_borrow = now - source->first_borrow_time;
        int time_since_last_borrow = now - source->last_borrow_time;

        if(time_since_first_borrow > source->disconnect_timeout_ms)
        {
            // If no lent message was received after the disconnect timeout,
            // enter unrecoverable error state
            source->state = ATOLLA_SOURCE_STATE_ERROR;
        }
        else if(time_since_last_borrow > source->retry_timeout_ms)
        {
            // If no lent message was received after the retry timeout, try borrowing again
            source_send_borrow(source);
        }
    }
}

static void source_iterate_recv_buf(AtollaSourcePrivate* source, size_t received_bytes)
{
    MsgIter iter = msg_iter_make(source->recv_buf, received_bytes);

    for(; msg_iter_has_msg(&iter); msg_iter_next(&iter))
    {
        MsgType type = msg_iter_type(&iter);

        switch(type)
        {
            case MSG_TYPE_LENT:
            {
                source_lent(source);
                break;
            }

            case MSG_TYPE_FAIL:
            {
                source_fail(source);
                break;
            }

            default:
            {
                assert(false);
                break;
            }
        }
    }
}

static void source_lent(AtollaSourcePrivate* source)
{
    source->state = ATOLLA_SOURCE_STATE_OPEN;
    source->last_frame_time = -1;
}

static void source_fail(AtollaSourcePrivate* source)
{
    source->state = ATOLLA_SOURCE_STATE_ERROR;
}
