#include "atolla/source.h"
#include "udp_socket/udp_socket.h"

#include <stdlib.h>

#include "test/assert.h"
#include "atolla/sleep_ms.h"

static const size_t recv_buf_len = 3 + 65537;
static const size_t send_buf_len = 3 + 65537;

struct AtollaSourcePrivate
{
    AtollaSourceState state;
    void* recv_buf;
    void* send_buf;
    UdpSocket sock;
    int frame_length_ms;
    int max_buffered_frames;
    int lag_ms; // How many ms are we behind schedule in sending frames
};
typedef struct AtollaSourcePrivate AtollaSourcePrivate;

AtollaSource atolla_source_make(const char* sink_hostname, int sink_port, int frame_length_ms, int max_buffered_frames)
{
    AtollaSourcePrivate* source = (AtollaSourcePrivate*) malloc(sizeof(AtollaSourcePrivate));
    source->state = ATOLLA_SOURCE_STATE_WAITING;
    source->recv_buf = malloc(recv_buf_len);
    source->send_buf = malloc(send_buf_len);
    source->frame_length_ms = frame_length_ms;
    source->max_buffered_frames = max_buffered_frames;
    source->lag_ms = max_buffered_frames * frame_length_ms;

    UdpSocketResult result;
    
    result = udp_socket_init(&source->sock);
    assert(result.code == UDP_SOCKET_OK);

    result = udp_socket_set_receiver(&source->sock, sink_hostname, (unsigned short) sink_port);
    assert(result.code == UDP_SOCKET_OK);

    AtollaSource source_handle = { source };
    return source_handle;
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

    return source->state;
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

    if(source->lag_ms < 0)
    {
        sleep_ms(source->lag_ms);
    }

    // TODO send the enqueued frame

    source->lag_ms -= source->frame_length_ms;

    return true;
}
