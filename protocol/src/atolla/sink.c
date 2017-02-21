#include "atolla/sink.h"
#include "msg/iter.h"
#include "udp_socket/udp_socket.h"

#include "test/assert.h"

#include <stdlib.h>

static const size_t recv_buf_len = 3 + 65537;

static void update(AtollaSink* sink);
static void borrow(int frame_length_ms, size_t buffer_length);
static void enqueue(size_t frame_idx, MemBlock frame);

AtollaSink atolla_sink_make(int port)
{
    AtollaSink sink;
    
    UdpSocket socket;
    UdpSocketResult result = udp_socket_init_on_port(&socket, (unsigned short) port);
    assert(result.code == UDP_SOCKET_OK);

    sink.state = ATOLLA_SINK_STATE_OPEN;
    sink.socket_handle = socket.socket_handle;
    sink.recv_buf = malloc(recv_buf_len);
    assert(sink.recv_buf);

    return sink;
}

AtollaSinkState atolla_sink_state(AtollaSink* sink)
{
    return sink->state;
}

bool atolla_sink_get(AtollaSink* sink, void* frame, size_t frame_len)
{
    update(sink);
    return false;
}

static void update(AtollaSink* sink)
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
                    borrow(frame_len, buffer_len);
                    break;
                }

                case MSG_TYPE_ENQUEUE:
                {
                    uint8_t frame_idx = msg_iter_enqueue_frame_idx(&iter);
                    MemBlock frame = msg_iter_enqueue_frame(&iter);
                    enqueue(frame_idx, frame);
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

static void borrow(int frame_length_ms, size_t buffer_length)
{
    assert(false);
}

static void enqueue(size_t frame_idx, MemBlock frame)
{
    assert(false);
}
