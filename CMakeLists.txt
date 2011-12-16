cmake_minimum_required(VERSION 2.8)

project(kmsc-endpoints)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Werror -std=c99")
set(CUSTOM_PREFIX "kmsc")
set(INCLUDE_PREFIX "include/${CUSTOM_PREFIX}")
set(LIB_PREFIX "lib")

add_subdirectory(src)

add_subdirectory(test)

enable_testing()

add_test(test-endpoints1 test/test-endpoints)
add_test(test_sdp test/test_sdp)
add_test(test_rtp test/test_rtp)
add_test(test_rtp_connection test/test_rtp_connection)
