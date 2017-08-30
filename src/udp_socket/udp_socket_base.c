#include "udp_socket.h"
#include "udp_socket_results_internal.h"

UdpSocketResult udp_socket_init(UdpSocket* socket)
{
    // use any free port
    return udp_socket_init_on_port(socket, 0);
}
