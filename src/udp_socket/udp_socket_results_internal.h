#ifndef UDP_SOCKET_RESULTS_INTERNAL_H
#define UDP_SOCKET_RESULTS_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "udp_socket.h"

UdpSocketResult make_success_result();

UdpSocketResult make_err_result(UdpSocketResultCode code, const char* msg);

#ifdef __cplusplus
}
#endif

#endif
