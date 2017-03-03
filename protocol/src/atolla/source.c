#include "atolla/source.h"
#include "msg/builder.h"
#include "msg/iter.h"
#include "test/assert.h"
#include "time/now.h"
#include "time/sleep.h"
#include "udp_socket/udp_socket.h"

#include <stdlib.h>

static const size_t recv_buf_len = 3 + 65537;

struct AtollaSourcePrivate
{
    AtollaSourceState state;
    UdpSocket sock;
    void* recv_buf;
    
    MsgBuilder builder;

    int next_frame_idx;
    int frame_length_ms;
    int max_buffered_frames;
    int last_frame_time;
};
typedef struct AtollaSourcePrivate AtollaSourcePrivate;

static AtollaSourcePrivate* source_private_make(int frame_length_ms, int max_buffered_frames);
static void source_send_borrow(AtollaSourcePrivate* source);
static void source_update(AtollaSourcePrivate* source);
static void iterate_recv_buf(AtollaSourcePrivate* sink, size_t received_bytes);

AtollaSource atolla_source_make(const char* sink_hostname, int sink_port, int frame_length_ms, int max_buffered_frames)
{
    AtollaSourcePrivate* source = source_private_make(frame_length_ms, max_buffered_frames);

    msg_builder_init(&source->builder);

    UdpSocketResult result;
    
    result = udp_socket_init(&source->sock);
    assert(result.code == UDP_SOCKET_OK);

    result = udp_socket_set_receiver(&source->sock, sink_hostname, (unsigned short) sink_port);
    assert(result.code == UDP_SOCKET_OK);

    source_send_borrow(source);

    AtollaSource source_handle = { source };
    return source_handle;
}

static AtollaSourcePrivate* source_private_make(int frame_length_ms, int max_buffered_frames)
{
    AtollaSourcePrivate* source = (AtollaSourcePrivate*) malloc(sizeof(AtollaSourcePrivate));

    source->state = ATOLLA_SOURCE_STATE_WAITING;
    source->recv_buf = malloc(recv_buf_len);
    source->next_frame_idx = 0;
    source->frame_length_ms = frame_length_ms;
    source->max_buffered_frames = max_buffered_frames;
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
    return (time_now() - source->last_frame_time) / source->frame_length_ms;
}

/**
 * Tries to enqueue the given frame in the connected sink.
 *
 * The call might block for some time in order to wait for the device to catch
 * up displaying the previously sent frames.
 */
bool atolla_source_put(AtollaSource source_handle, void* frame, size_t frame_len)
{
    if(atolla_source_state(source_handle) != ATOLLA_SOURCE_STATE_OPEN)
    {
        return false;
    }

    AtollaSourcePrivate* source = (AtollaSourcePrivate*) source_handle.private;

    if(atolla_source_frame_lag(source_handle) == 0)
    {
        // If the receiving device has no space in the buffer to hold new frames,
        // wait for the duration of one frame
        time_sleep(source->frame_length_ms);
    }

    MemBlock* enqueue_msg = msg_builder_enqueue(&source->builder, source->next_frame_idx, frame, frame_len);

    udp_socket_send(&source->sock, enqueue_msg->data, enqueue_msg->size);

    ++source->next_frame_idx;
    source->last_frame_time += source->frame_length_ms;

    return true;
}

static void source_send_borrow(AtollaSourcePrivate* source)
{
    MemBlock* borrow_msg = msg_builder_borrow(&source->builder, source->frame_length_ms, source->max_buffered_frames);
    udp_socket_send(&source->sock, borrow_msg->data, borrow_msg->size);
}

static void source_update(AtollaSourcePrivate* source)
{   
    size_t received_len;
    UdpSocketResult result;

    result = udp_socket_receive(
        &source->sock,
        source->recv_buf, recv_buf_len,
        &received_len
    );

    if(result.code == UDP_SOCKET_OK)
    {
        iterate_recv_buf(source, received_len);
    }
}

static void iterate_recv_buf(AtollaSourcePrivate* source, size_t received_bytes)
{
    MsgIter iter = msg_iter_make(source->recv_buf, received_bytes);

    while(msg_iter_has_msg(&iter))
    {
        MsgType type = msg_iter_type(&iter);

        switch(type)
        {
            case MSG_TYPE_LENT:
            {
                source->state = ATOLLA_SOURCE_STATE_OPEN;
                source->last_frame_time = time_now() - source->max_buffered_frames * source->frame_length_ms;
                break;
            }

            case MSG_TYPE_FAIL:
            {
                source->state = ATOLLA_SOURCE_STATE_ERROR;
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