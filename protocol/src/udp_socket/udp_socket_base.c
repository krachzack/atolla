#include "udp_socket.h"
#include "udp_socket_results_internal.h"

#ifndef HAVE_ARDUINO_WIFI_UDP
#include "atolla/config.h"
#endif

#if !defined(HAVE_POSIX_SOCKETS) && !defined(HAVE_WINSOCK2) && !defined(HAVE_ARDUINO_WIFI_UDP)
#error "No configuration symbol for available backend defined"
#endif

UdpSocketResult udp_socket_init(UdpSocket* socket)
{
    // use any free port
    return udp_socket_init_on_port(socket, 0);
}
