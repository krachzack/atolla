#if defined(ARDUINO_ARCH_ESP8266)

#include <Arduino.h>
#include <WifiUdp.h>
#include "test/assert.h"
#include "udp_socket_results_internal.h"
#include "udp_socket_messages.h"

static WiFiUDP Udp;
static bool has_receiver;
static uint16_t receiver_port;
static IPAddress receiver;

UdpSocketResult udp_socket_init_on_port(UdpSocket* socket, unsigned short port)
{
    if(socket == NULL)
    {
        return make_err_result(
            UDP_SOCKET_ERR_SOCKET_IS_NULL,
            msg_socket_is_null
        );
    }

    // Intialize everything to zero, false, by standard is always 0
    memset(socket, 0, sizeof(UdpSocket));

    Udp.begin(port);
    receiver_port = 0;
    has_receiver = false;

    return make_success_result();
}

UdpSocketResult udp_socket_free(UdpSocket* socket)
{
    if(socket == NULL)
    {
        return make_err_result(
            UDP_SOCKET_ERR_SOCKET_IS_NULL,
            msg_socket_is_null
        );
    }

    Udp.stop();

    // Overwrite the handle since it is not valid anymore
    socket->socket_handle = -1;

    return make_success_result();
}

UdpSocketResult udp_socket_set_receiver(UdpSocket* socket, const char* hostname, unsigned short port)
{
    assert(false); // Not implemented yet
}

UdpSocketResult udp_socket_send(UdpSocket* socket, void* packet_data, size_t packet_data_len)
{
    assert(packet_data != NULL);
    assert(packet_data_len > 0);

    if(socket == NULL)
    {
        return make_err_result(
            UDP_SOCKET_ERR_SOCKET_IS_NULL,
            msg_socket_is_null
        );
    }

    if(!has_receiver) {
        return make_err_result(
            UDP_SOCKET_ERR_NO_RECEIVER,
            msg_no_receiver
        );
    }

    Udp.beginPacket(receiver, receiver_port);
    Udp.write((const uint8_t*) packet_data, packet_data_len);
    Udp.endPacket();

    return make_success_result();
}

/**
 * TODO document
 *
 * then_respond makes that sends always send to the partner last received from.
 *
 * The result of the operation will be signalled with the returned UdpSocketResult
 * structure. A successful completion will be signalled with the <code>code</code>
 * property being set to <code>UDP_SOCKET_OK</code>, which is equivalent to
 * the integer <code>0</code>. If the operation failed, the error will be
 * specified with <code>code</code> being set to any of the
 * <code>UDP_SOCKET_ERR_*</code> values and the <code>msg</code> property pointing
 * to a human readable error message.
 */
UdpSocketResult udp_socket_receive(UdpSocket* socket, void* packet_data, size_t max_packet_size, size_t* received_byte_count, bool then_respond)
{
    assert(packet_data != NULL);
    assert(max_packet_size > 0);

    int packetSize = Udp.parsePacket();
    if (packetSize)
    {
        int bytes_read = Udp.read((unsigned char*) packet_data, max_packet_size);

        if(received_byte_count)
        {
            *received_byte_count = bytes_read;
        }

        if(then_respond) {
            has_receiver = true;
            receiver = Udp.remoteIP();
            receiver_port = Udp.remotePort();
        }

        return make_success_result();
    } else {
        if(received_byte_count)
        {
            *received_byte_count = 0;
        }

        return make_err_result(
            UDP_SOCKET_ERR_NOTHING_RECEIVED,
            msg_nothing_received
        );
    }
}

#endif
