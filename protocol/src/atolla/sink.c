#include "atolla/sink.h"
#include "msg/builder.h"
#include "msg/iter.h"
#include "udp_socket/udp_socket.h"
#include "time/now.h"
#include "test/assert.h"

#include <stdlib.h>
#include <string.h>

struct AtollaSinkPrivate
{
    AtollaSinkState state;

    UdpSocket socket;

    int lights_count;
    int frame_duration_ms;

    MsgBuilder builder;

    MemBlock recv_buf;
    MemBlock frame_buf;

    int first_enqueue_time;
};
typedef struct AtollaSinkPrivate AtollaSinkPrivate;

static const size_t recv_buf_len = 3 + 65537;

static AtollaSinkPrivate* sink_private_make(const AtollaSinkSpec* spec); 
static void sink_iterate_recv_buf(AtollaSinkPrivate* sink);
static void sink_handle_borrow(AtollaSinkPrivate* sink, int frame_length_ms, size_t buffer_length);
static void sink_handle_enqueue(AtollaSinkPrivate* sink, size_t frame_idx, MemBlock frame);
static void sink_send_lent(AtollaSinkPrivate* sink);
static void sink_update(AtollaSinkPrivate* sink);

AtollaSink atolla_sink_make(const AtollaSinkSpec* spec)
{
    AtollaSinkPrivate* sink = sink_private_make(spec);
    
    UdpSocketResult result = udp_socket_init_on_port(&sink->socket, (unsigned short) spec->port);
    assert(result.code == UDP_SOCKET_OK);

    msg_builder_init(&sink->builder);

    AtollaSink sink_handle = { sink };
    return sink_handle;
}

static AtollaSinkPrivate* sink_private_make(const AtollaSinkSpec* spec)
{
    assert(spec->port >= 0 && spec->port < 65536);
    assert(spec->lights_count >= 1);

    AtollaSinkPrivate* sink = (AtollaSinkPrivate*) malloc(sizeof(AtollaSinkPrivate));
    assert(sink);

    // Initialize everything to zero
    memset(sink, 0, sizeof(AtollaSinkPrivate));

    // Except these fields, which are pre-filled
    sink->state = ATOLLA_SINK_STATE_OPEN;
    sink->lights_count = spec->lights_count;
    sink->recv_buf = mem_block_alloc(recv_buf_len);

    return sink;
}

void atolla_sink_free(AtollaSink sink_handle)
{
    AtollaSinkPrivate* sink = (AtollaSinkPrivate*) sink_handle.private;

    UdpSocketResult result = udp_socket_free(&sink->socket);
    assert(result.code == UDP_SOCKET_OK);

    mem_block_free(&sink->recv_buf);

    if(sink->frame_buf.data != NULL)
    {
        mem_block_free(&sink->frame_buf);
    }

    free(sink);
}

AtollaSinkState atolla_sink_state(AtollaSink sink_handle)
{
    AtollaSinkPrivate* sink = (AtollaSinkPrivate*) sink_handle.private;

    sink_update(sink);

    return sink->state;
}

bool atolla_sink_get(AtollaSink sink_handle, void* frame, size_t frame_len)
{
    AtollaSinkPrivate* sink = (AtollaSinkPrivate*) sink_handle.private;

    sink_update(sink);

    bool lent = sink->state == ATOLLA_SINK_STATE_LENT;

    if(lent)
    {
        const size_t frame_size = sink->lights_count * 3;
        assert(frame_len >= frame_size);

        const size_t frame_buf_frame_count = sink->frame_buf.size / frame_size;

        // FIXME rather than lent time, the reference should be the first enqueued frame

        size_t frame_idx = ((time_now() - sink->first_enqueue_time) / sink->frame_duration_ms) % frame_buf_frame_count;
        
        void* frame_buf_frame = ((uint8_t*) sink->frame_buf.data) + (frame_idx * frame_size);

        memcpy(frame, frame_buf_frame, frame_size);
    }

    return lent;
}

static void sink_update(AtollaSinkPrivate* sink)
{
    UdpSocketResult result;

    result = udp_socket_receive(
        &sink->socket,
        sink->recv_buf.data, sink->recv_buf.capacity,
        &sink->recv_buf.size,
        true
    );

    if(result.code == UDP_SOCKET_OK)
    {
        sink_iterate_recv_buf(sink);
    }
}

static void sink_iterate_recv_buf(AtollaSinkPrivate* sink)
{
    MsgIter iter = msg_iter_make(sink->recv_buf.data, sink->recv_buf.size);

    for(; msg_iter_has_msg(&iter); msg_iter_next(&iter))
    {
        MsgType type = msg_iter_type(&iter);

        switch(type)
        {
            case MSG_TYPE_BORROW:
            {
                uint8_t frame_len = msg_iter_borrow_frame_length(&iter);
                uint8_t buffer_len = msg_iter_borrow_buffer_length(&iter);
                sink_handle_borrow(sink, frame_len, buffer_len);
                break;
            }

            case MSG_TYPE_ENQUEUE:
            {
                uint8_t frame_idx = msg_iter_enqueue_frame_idx(&iter);
                MemBlock frame = msg_iter_enqueue_frame(&iter);
                sink_handle_enqueue(sink, frame_idx, frame);
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

static void sink_handle_borrow(AtollaSinkPrivate* sink, int frame_length_ms, size_t buffer_length)
{
    assert(buffer_length >= 0);
    assert(frame_length_ms >= 0);

    if(sink->frame_buf.data)
    {
        mem_block_free(&sink->frame_buf);
    }

    // Create the frame buffer and initialize it to zero
    // Consider it full by setting size to capacity
    size_t required_frame_buf_size = buffer_length * (sink->lights_count * 3);
    sink->frame_buf = mem_block_alloc(required_frame_buf_size);
    memset(sink->frame_buf.data, 0, sink->frame_buf.capacity);
    sink->frame_buf.size = sink->frame_buf.capacity; 

    sink->frame_duration_ms = frame_length_ms;
    sink->state = ATOLLA_SINK_STATE_LENT;

    sink_send_lent(sink);
}

static void sink_send_lent(AtollaSinkPrivate* sink)
{
    MemBlock* lent_msg = msg_builder_lent(&sink->builder);
    udp_socket_send(&sink->socket, lent_msg->data, lent_msg->size);
    sink->first_enqueue_time = -1;
}

static void sink_handle_enqueue(AtollaSinkPrivate* sink, size_t frame_idx, MemBlock frame)
{
    const size_t frame_size = sink->lights_count * 3;

    const size_t frame_buf_frame_count = sink->frame_buf.size / frame_size;
    assert(frame_idx >= 0 && frame_idx < frame_buf_frame_count);
    assert(frame.size >= 3);

    MemBlock frame_buf_indexed_frame = mem_block_slice(
        &sink->frame_buf,
        frame_idx * frame_size,
        frame_size
    );

    uint8_t* frame_buf_bytes = (uint8_t*) frame_buf_indexed_frame.data;
    uint8_t* frame_input_bytes = (uint8_t*) frame.data;

    for(int light_idx = 0; light_idx < sink->lights_count; ++light_idx)
    {
        size_t offset = light_idx * 3;
        frame_buf_bytes[offset + 0] = frame_input_bytes[(offset + 0) % frame.size];
        frame_buf_bytes[offset + 1] = frame_input_bytes[(offset + 1) % frame.size];
        frame_buf_bytes[offset + 2] = frame_input_bytes[(offset + 2) % frame.size];
    }

    if(sink->first_enqueue_time == -1)
    {
        sink->first_enqueue_time = time_now();
    }
}
