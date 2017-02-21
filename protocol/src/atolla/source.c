#include "atolla/source.h"
#include "udp_socket/udp_socket.h"

#include <stdlib.h>

#include "test/assert.h"

static const size_t recv_buf_len = 3 + 65537;
static const size_t send_buf_len = 3 + 65537;

AtollaSource atolla_source_make(const char* sink_hostname, int sink_port)
{
    AtollaSource source;
    source.state = ATOLLA_SOURCE_STATE_WAITING;
    source.recv_buf = malloc(recv_buf_len);
    source.send_buf = malloc(send_buf_len);

    UdpSocket sock;
    UdpSocketResult result;
    
    result = udp_socket_init(&sock);
    assert(result.code == UDP_SOCKET_OK);

    result = udp_socket_set_receiver(&sock, sink_hostname, (unsigned short) sink_port);
    assert(result.code == UDP_SOCKET_OK);

    source.socket_handle = sock.socket_handle;

    return source;
}

AtollaSourceState atolla_source_state(AtollaSource* source)
{
    return source->state;
}

/**
 * Tries to enqueue the given frame in the connected sink.
 *
 * The call might block for some time in order to wait for the device to catch
 * up displaying the previously sent frames.
 */
bool atolla_source_put(AtollaSource* source, void* frame, size_t frame_len)
{
    assert(false);
    return false;
}
