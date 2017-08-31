#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
extern "C" {
    #include <cmocka.h>
}
#include "udp_socket/udp_socket.h"
#include "time/sleep.h"

static void test_init_null(void** state)
{
    UdpSocketResult result;

    result = udp_socket_init(NULL);
    assert_int_equal(result.code, UDP_SOCKET_ERR_SOCKET_IS_NULL);

    result = udp_socket_init_on_port(NULL, 0);
    assert_int_equal(result.code, UDP_SOCKET_ERR_SOCKET_IS_NULL);
}

/* A test case that does nothing and succeeds. */
static void test_init_and_free_any_port(void** state)
{
    UdpSocket socket;
    UdpSocketResult result = udp_socket_init(&socket);

    assert_int_equal(result.code, UDP_SOCKET_OK);

    result = udp_socket_free(&socket);
    assert_int_equal(result.code, UDP_SOCKET_OK);
}

/* A test case that does nothing and succeeds. */
static void test_init_and_free_on_port(void** state)
{
    UdpSocket socket;
    UdpSocketResult result = udp_socket_init_on_port(&socket, 1800);

    assert_int_equal(result.code, UDP_SOCKET_OK);

    result = udp_socket_free(&socket);

    assert_int_equal(result.code, UDP_SOCKET_OK);
}

static void test_init_twice_on_same_port(void** state)
{
    unsigned short same_port = 24213;

    UdpSocket socket1;
    UdpSocket socket2;
    UdpSocketResult result;

    result = udp_socket_init_on_port(&socket1, same_port);
    assert_int_equal(result.code, UDP_SOCKET_OK);

    result = udp_socket_init_on_port(&socket2, same_port);
    assert_int_equal(result.code, UDP_SOCKET_ERR_PORT_IN_USE);

    result = udp_socket_free(&socket1);
    assert_int_equal(result.code, UDP_SOCKET_OK);
}

static void test_connect_valid_hostname(void** state)
{
    UdpSocket socket;
    UdpSocketResult result = udp_socket_init(&socket);

    assert_int_equal(result.code, UDP_SOCKET_OK);

    result = udp_socket_set_receiver(&socket, "localhost", 8080);
    assert_int_equal(result.code, UDP_SOCKET_OK);

    result = udp_socket_free(&socket);
    assert_int_equal(result.code, UDP_SOCKET_OK);
}

static void test_connect_invalid_hostname(void** state)
{
    UdpSocket socket;
    UdpSocketResult result = udp_socket_init(&socket);

    assert_int_equal(result.code, UDP_SOCKET_OK);

    result = udp_socket_set_receiver(&socket, "wrdlbrnft12141412414", 8080);
    assert_int_equal(result.code, UDP_SOCKET_ERR_RESOLVE_HOSTNAME_FAILED);

    result = udp_socket_free(&socket);
    assert_int_equal(result.code, UDP_SOCKET_OK);
}

static void test_send_and_receive(void** state)
{
    unsigned short port1 = 24000;
    unsigned short port2 = 48000;

    UdpSocket socket1;
    UdpSocket socket2;
    UdpSocketResult result;
    unsigned long data = 0xDEADBEEF;
    unsigned long received = ~data;
    size_t received_bytes = 42;

    result = udp_socket_init_on_port(&socket1, port1);
    assert_int_equal(result.code, UDP_SOCKET_OK);

    result = udp_socket_init_on_port(&socket2, port2);
    assert_int_equal(result.code, UDP_SOCKET_OK);

    result = udp_socket_set_receiver(&socket1, "127.0.0.1", port2);
    assert_int_equal(result.code, UDP_SOCKET_OK);

    udp_socket_set_receiver(&socket2, "127.0.0.1", port1);
    assert_int_equal(result.code, UDP_SOCKET_OK);

    result = udp_socket_send(&socket1, &data, sizeof(data));
    assert_int_equal(result.code, UDP_SOCKET_OK);

    result = udp_socket_receive(&socket2, &received, sizeof(received), &received_bytes, false);

    /** Receiving is expected to take some time, since sockets are non-blocking */
    assert_int_equal(result.code, UDP_SOCKET_ERR_NOTHING_RECEIVED);
    assert_int_equal(received_bytes, 0);

    time_sleep(500);
    result = udp_socket_receive(&socket2, &received, sizeof(received), &received_bytes, false);

    assert_int_equal(result.code, UDP_SOCKET_OK);
    assert_int_equal(received_bytes, sizeof(received));
    assert_int_equal(data, received);

    result = udp_socket_send(&socket1, &data, sizeof(data));
    assert_int_equal(result.code, UDP_SOCKET_OK);

    time_sleep(500);
    // received_bytes is optional
    result = udp_socket_receive(&socket2, &received, sizeof(received), NULL, false);
    assert_int_equal(result.code, UDP_SOCKET_OK);

    result = udp_socket_free(&socket1);
    assert_int_equal(result.code, UDP_SOCKET_OK);

    result = udp_socket_free(&socket2);
    assert_int_equal(result.code, UDP_SOCKET_OK);
}

static void test_send_with_no_receiver(void** state)
{
    UdpSocket socket;
    UdpSocketResult result = udp_socket_init(&socket);

    assert_int_equal(result.code, UDP_SOCKET_OK);
    int msg = 42;

    result = udp_socket_send(&socket, &msg, sizeof(msg));
    assert_int_equal(result.code, UDP_SOCKET_ERR_NO_RECEIVER);

    result = udp_socket_free(&socket);
    assert_int_equal(result.code, UDP_SOCKET_OK);
}

static void test_disconnect(void** state)
{
    UdpSocket socket;
    UdpSocketResult result = udp_socket_init(&socket);

    assert_int_equal(result.code, UDP_SOCKET_OK);
    int msg = 42;

    udp_socket_set_receiver(&socket, "127.0.0.1", 10000);
    result = udp_socket_send(&socket, &msg, sizeof(msg));
    assert_int_equal(result.code, UDP_SOCKET_OK);

    udp_socket_set_endpoint(&socket, NULL);
    result = udp_socket_send(&socket, &msg, sizeof(msg));
    assert_int_equal(result.code, UDP_SOCKET_ERR_NO_RECEIVER);

    result = udp_socket_free(&socket);
    assert_int_equal(result.code, UDP_SOCKET_OK);
}


int main(int argc, char* argv[])
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_init_null),
        cmocka_unit_test(test_init_and_free_any_port),
        cmocka_unit_test(test_init_and_free_on_port),
        cmocka_unit_test(test_init_twice_on_same_port),
        cmocka_unit_test(test_connect_valid_hostname),
        cmocka_unit_test(test_connect_invalid_hostname),
        cmocka_unit_test(test_send_and_receive),
        cmocka_unit_test(test_send_with_no_receiver),
        cmocka_unit_test(test_disconnect)
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
