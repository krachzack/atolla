cmake_minimum_required (VERSION 2.6)
project (Atolla)

#
# Specify cmake module path
#
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/")


#
# Include custom modules
#
include (AddCMockaTest)

#
# Include standard modules
#
include (CheckSymbolExists)
include (CheckIncludeFiles)
include (CheckTypeSize)
include (TestBigEndian)

set (ATOLLA_VERSION_MAJOR 1)
set (ATOLLA_VERSION_MINOR 0)
set (ATOLLA_VERSION_PATCH 0)


#
# Check for headers to native bsd-style sockets
#

check_include_files ("winsock2.h" HAVE_WINSOCK2)
check_include_files ("sys/socket.h;netinet/in.h;fcntl.h;netdb.h" HAVE_POSIX_SOCKETS)

#
# Make sure headers for special types are available
#

SET(CMAKE_EXTRA_INCLUDE_FILES stddef.h)
CHECK_TYPE_SIZE("size_t" SIZE_T)
SET(CMAKE_EXTRA_INCLUDE_FILES)

SET(CMAKE_EXTRA_INCLUDE_FILES stdbool.h)
CHECK_TYPE_SIZE("bool" BOOL)
SET(CMAKE_EXTRA_INCLUDE_FILES)

SET(CMAKE_EXTRA_INCLUDE_FILES stdint.h)
CHECK_TYPE_SIZE("uint8_t" UINT8_T)
CHECK_TYPE_SIZE("uint16_t" UINT16_T)
SET(CMAKE_EXTRA_INCLUDE_FILES)

#
# Check for standard library functions
#

check_symbol_exists (memset "string.h" HAVE_MEMSET)
check_symbol_exists (memcpy "string.h" HAVE_MEMCPY)
check_symbol_exists (malloc "stdlib.h" HAVE_MALLOC)

check_symbol_exists (Sleep "windows.h" HAVE_WINDOWS_SLEEP)
check_symbol_exists (usleep "unistd.h" HAVE_POSIX_SLEEP)

#
# Check endianness
#

test_big_endian(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
    set(NATIVE_BIG_ENDIAN 1)
    set(NATIVE_LITTLE_ENDIAN 0)
else()
    set(NATIVE_BIG_ENDIAN 0)
    set(NATIVE_LITTLE_ENDIAN 1)
endif()


#
# Set include directories
#
include_directories(src ${CMOCKA_INCLUDE})

#
# Copy public headers to build directory
#
configure_file(src/atolla/primitives.h ${CMAKE_CURRENT_BINARY_DIR}/include/atolla/primitives.h COPYONLY)
configure_file(src/atolla/sink.h ${CMAKE_CURRENT_BINARY_DIR}/include/atolla/sink.h COPYONLY)
configure_file(src/atolla/source.h ${CMAKE_CURRENT_BINARY_DIR}/include/atolla/source.h COPYONLY)

#
# Define source files for building
#
set(
    LIBRARY_HEADERS
    src/atolla/primitives.h
    src/atolla/sink.h
    src/atolla/source.h
    src/atolla/error_codes.h
    src/mem/block.h
    src/mem/ring.h
    src/mem/uint16_byte.h
    src/mem/uint16le.h
    src/msg/builder.h
    src/msg/iter.h
    src/msg/type.h
    src/test/assert.h
    src/time/gettime.h
    src/time/mach_gettime.h
    src/time/now.h
    src/time/sleep.h
    src/udp_socket/sockets_headers.h
    src/udp_socket/udp_socket_messages.h
    src/udp_socket/udp_socket_results_internal.h
    src/udp_socket/udp_socket.h
)
set(LIBRARY_IMPLS
    src/atolla/sink.c
    src/atolla/source.c
    src/mem/block.c
    src/mem/ring.c
    src/msg/builder.c
    src/msg/iter.c
    src/udp_socket/udp_socket_base.c
    src/udp_socket/udp_socket_bsdlike.c
    src/udp_socket/udp_socket_results_internal.c
    src/udp_socket/udp_socket_wifiudp.cpp
    src/time/mach_gettime.c
    src/time/now.c
)
set(LIBRARY_SRC ${LIBRARY_HEADERS} ${LIBRARY_IMPLS})


#
# Add build tasks
#

add_library(atolla STATIC ${LIBRARY_SRC})
add_library(atolla.${ATOLLA_VERSION_MAJOR}.${ATOLLA_VERSION_MINOR}.${ATOLLA_VERSION_PATCH} SHARED ${LIBRARY_SRC})

add_executable(example_yellow examples/01-yellow.c)
add_executable(example_sine   examples/02-sine.c)
add_executable(example_remote examples/03-remote.c)

target_link_libraries(example_yellow atolla)
add_custom_target(run_example_yellow DEPENDS example_yellow COMMAND ../build/example_yellow)

target_link_libraries(example_sine atolla)
add_custom_target(run_example_sine DEPENDS example_sine COMMAND ../build/example_sine)

target_link_libraries(example_remote atolla)
add_custom_target(run_example_remote DEPENDS example_remote COMMAND ../build/example_remote)

add_cmocka_test(mem_ring_tests       tests/mem_ring_tests.c       ${LIBRARY_SRC})
add_cmocka_test(msg_builder_tests    tests/msg_builder_tests.c    ${LIBRARY_SRC})
add_cmocka_test(msg_iter_tests       tests/msg_iter_tests.c       ${LIBRARY_SRC})
add_cmocka_test(sink_tests           tests/sink_tests.c           ${LIBRARY_SRC})
add_cmocka_test(source_tests         tests/source_tests.c         ${LIBRARY_SRC})
add_cmocka_test(source_to_sink_tests tests/source_to_sink_tests.c ${LIBRARY_SRC})
add_cmocka_test(time_tests           tests/time_tests.c           ${LIBRARY_SRC})
add_cmocka_test(udp_socket_tests     tests/udp_socket_tests.c     ${LIBRARY_SRC})

add_custom_target(test_all DEPENDS mem_ring_tests msg_builder_tests msg_iter_tests sink_tests source_tests time_tests udp_socket_tests COMMAND ../test)
