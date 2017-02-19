#ifndef _sockets_headers_h_
#define _sockets_headers_h_

#include "atolla/config.h"

#if defined(HAVE_POSIX_SOCKETS)
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <fcntl.h>
    #include <netdb.h>
#elif defined(HAVE_WINSOCK2)
    #pragma comment( lib, "wsock32.lib" )
    #include <winsock2.h>
#else
    #error "Neither Posix Sockets nor Windows Sockets could be found by the build system"
#endif

#endif /* _sockets_headers_h_ */
