cmake_minimum_required (VERSION 2.6)
project (Atolla)

find_package(OpenGL)

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
include (CheckCXXCompilerFlag)


set (ATOLLA_VERSION_MAJOR 1)
set (ATOLLA_VERSION_MINOR 0)
set (ATOLLA_VERSION_PATCH 0)

function(enable_cxx_compiler_flag_if_supported flag)
    string(FIND "${CMAKE_CXX_FLAGS}" "${flag}" flag_already_set)
    if(flag_already_set EQUAL -1)
        check_cxx_compiler_flag("${flag}" flag_supported)
        if(flag_supported)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
        endif()
        unset(flag_supported CACHE)
    endif()
endfunction()

#
# Show me your worst warnings for C++
#
enable_cxx_compiler_flag_if_supported("-Wall")
enable_cxx_compiler_flag_if_supported("-pedantic")
enable_cxx_compiler_flag_if_supported("-Wsign-compare")

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
configure_file(src/atolla/version.h ${CMAKE_CURRENT_BINARY_DIR}/include/atolla/version.h COPYONLY)

#
# Define source files for building
#
set(
    LIBRARY_HEADERS
    src/atolla/primitives.h
    src/atolla/sink.h
    src/atolla/source.h
    src/atolla/version.h
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
    src/atolla/sink.cpp
    src/atolla/source.cpp
    src/mem/block.c
    src/mem/ring.c
    src/msg/builder.c
    src/msg/iter.c
    src/udp_socket/udp_socket_base.cpp
    src/udp_socket/udp_socket_bsdlike.cpp
    src/udp_socket/udp_socket_results_internal.cpp
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

if(OPENGL_FOUND)
    # Add GLFW for sink example
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(vendor/glfw)

    add_executable(example_sink examples/01-sink.cpp)
    include_directories(${OPENGL_INCLUDE_DIR})
    target_link_libraries(example_sink ${OPENGL_gl_LIBRARY})
    target_link_libraries(example_sink glfw atolla)
endif()

add_executable(example_yellow examples/02-yellow.cpp)
target_link_libraries(example_yellow atolla)

add_executable(example_sine   examples/03-sine.cpp)
target_link_libraries(example_sine atolla)

add_executable(example_complementary   examples/04-complementary.cpp)
target_link_libraries(example_complementary atolla)

add_cmocka_test(mem_ring_tests       tests/mem_ring_tests.cpp       ${LIBRARY_SRC})
add_cmocka_test(msg_builder_tests    tests/msg_builder_tests.cpp    ${LIBRARY_SRC})
add_cmocka_test(msg_iter_tests       tests/msg_iter_tests.cpp       ${LIBRARY_SRC})
add_cmocka_test(sink_tests           tests/sink_tests.cpp           ${LIBRARY_SRC})
add_cmocka_test(source_tests         tests/source_tests.cpp         ${LIBRARY_SRC})
add_cmocka_test(source_to_sink_tests tests/source_to_sink_tests.cpp ${LIBRARY_SRC})
add_cmocka_test(time_tests           tests/time_tests.cpp           ${LIBRARY_SRC})
add_cmocka_test(udp_socket_tests     tests/udp_socket_tests.cpp     ${LIBRARY_SRC})

add_custom_target(test_pretty DEPENDS mem_ring_tests msg_builder_tests msg_iter_tests sink_tests source_tests source_to_sink_tests time_tests udp_socket_tests COMMAND ../test)
