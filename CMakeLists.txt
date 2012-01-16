cmake_minimum_required(VERSION 2.8)

project(kmsc-endpoints)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Werror -std=c99")
set(CUSTOM_PREFIX "kmsc")
set(INCLUDE_PREFIX "include/${CUSTOM_PREFIX}")
set(LIB_PREFIX "lib")

find_package(PkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0>=2.30)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-0.10>=0.10.35)

add_subdirectory(src)

add_subdirectory(test)

enable_testing()

add_test(test-endpoints1 test/test-endpoints)
add_test(test_sdp test/test_sdp)
add_test(test_rtp test/test_rtp)
add_test(test_rtp_connection test/test_rtp_connection)
add_test(test_rtp_transmission test/test_rtp_transmission)
add_test(test_mixer test/test_mixer)
