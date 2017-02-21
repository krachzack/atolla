#include "atolla/sink.h"
#include "msg/iter.h"
#include "udp_socket/udp_socket.h"

#include "test/assert.h"

#include <stdlib.h>
#include <string.h>

static const size_t recv_buf_len = 3 + 65537;

static void borrow(AtollaSink* sink, int frame_length_ms, size_t buffer_length);
static void enqueue(AtollaSink* sink, size_t frame_idx, MemBlock frame);

AtollaSink atolla_sink_make(int udp_port, int lights_count)
{
    AtollaSink sink;
    
    UdpSocket socket;
    UdpSocketResult result = udp_socket_init_on_port(&socket, (unsigned short) udp_port);
    assert(result.code == UDP_SOCKET_OK);

    sink.state = ATOLLA_SINK_STATE_OPEN;
    sink.lights_count = lights_count;
    sink.socket_handle = socket.socket_handle;
    sink.recv_buf = malloc(recv_buf_len);
    sink.frame_buf = NULL;
    sink.frame_buf_len = 0;

    assert(sink.lights_count >= 1);
    assert(sink.recv_buf);

    return sink;
}

AtollaSinkState atolla_sink_state(AtollaSink* sink)
{
    return sink->state;
}

bool atolla_sink_get(AtollaSink* sink, void* frame, size_t frame_len)
{
    bool lent = sink->state == ATOLLA_SINK_STATE_LENT;

    if(lent)
    {
        const size_t frame_size = sink->lights_count * 3;
        assert(frame_len >= frame_size);

        // FIXME this should be determined by time, not fixed at 1
        size_t frame_idx = 1;
        
        void* frame_buf_frame = ((uint8_t*) sink->frame_buf) + (frame_idx * frame_size);

        memcpy(frame, frame_buf_frame, frame_size);
    }

    return lent;
}

void atolla_sink_update(AtollaSink* sink)
{
    UdpSocket sock = { sink->socket_handle };
    size_t received_len;
    UdpSocketResult result;

    result = udp_socket_receive(
        &sock,
        sink->recv_buf, recv_buf_len,
        &received_len
    );

    if(result.code == UDP_SOCKET_OK)
    {
        MsgIter iter = msg_iter_make(sink->recv_buf, received_len);
        while(msg_iter_has_msg(&iter))
        {
            MsgType type = msg_iter_type(&iter);

            switch(type)
            {
                case MSG_TYPE_BORROW:
                {
                    uint8_t frame_len = msg_iter_borrow_frame_length(&iter);
                    uint8_t buffer_len = msg_iter_borrow_buffer_length(&iter);
                    borrow(sink, frame_len, buffer_len);
                    break;
                }

                case MSG_TYPE_ENQUEUE:
                {
                    uint8_t frame_idx = msg_iter_enqueue_frame_idx(&iter);
                    MemBlock frame = msg_iter_enqueue_frame(&iter);
                    enqueue(sink, frame_idx, frame);
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
}

static void borrow(AtollaSink* sink, int frame_length_ms, size_t buffer_length)
{
    assert(buffer_length >= 0);
    assert(frame_length_ms >= 0);

    if(sink->frame_buf)
    {
        free(sink->frame_buf);
    }

    sink->frame_buf_len = buffer_length * (sink->lights_count * 3);
    sink->frame_buf = malloc(sink->frame_buf_len);
    sink->frame_duration_ms = frame_length_ms;
    sink->state = ATOLLA_SINK_STATE_LENT;
}

static void enqueue(AtollaSink* sink, size_t frame_idx, MemBlock frame)
{
    const size_t frame_size = sink->lights_count * 3;
    const size_t frame_buf_frame_count = sink->frame_buf_len / frame_size;
    assert(frame_idx >= 0 && frame_idx < frame_buf_frame_count);
    assert(frame.size >= 3);

    MemBlock frame_buf = mem_block_make(sink->frame_buf, sink->frame_buf_len);
    MemBlock frame_buf_indexed_frame = mem_block_slice(
        &frame_buf,
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
}
