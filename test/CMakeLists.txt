cmake_minimum_required(VERSION 2.8)

add_library(test_memory memory.c)

add_executable(test-endpoints main.c)
add_executable(test_sdp test_sdp.c)
add_executable(test_rtp test_rtp.c )
add_executable(test_rtp_connection test_rtp_connection.c)
add_executable(test_rtp_transmission test_rtp_transmission.c)
add_executable(test_mixer test_mixer.c)
add_executable(test_rtmp test_rtmp.c)

find_package(PkgConfig)

include_directories(test-endpoints ${GLIB2_INCLUDE_DIRS})
target_link_libraries(test-endpoints ${GLIB2_LIBRARIES})

include_directories(test-endpoints ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(test-endpoints ${GSTREAMER_LIBRARIES})

target_link_libraries(test-endpoints kmsc-test-endpoints)

include_directories(test-endpoints ${kmsc_SOURCE_DIR}/src/core)
include_directories(test-endpoints kmsc-test-endpoints)

target_link_libraries(test_sdp kmsclib test_memory)
include_directories(test_sdp ${kmsc_SOURCE_DIR}/src/core)

target_link_libraries(test_rtp kmsclib test_memory rtp-test-lib)
include_directories(test_rtp ${kmsc_SOURCE_DIR}/src/core)
include_directories(test_rtp ${kmsc_SOURCE_DIR}/src)

target_link_libraries(test_rtp_connection kmsclib test_memory rtp-test-lib)
include_directories(test_rtp_connection ${kmsc_SOURCE_DIR}/src/core)
include_directories(test_rtp_connection ${kmsc_SOURCE_DIR}/src)

target_link_libraries(test_rtp_transmission kmsclib test_memory rtp-test-lib)
include_directories(test_rtp_transmission ${kmsc_SOURCE_DIR}/src/core)
include_directories(test_rtp_transmission ${kmsc_SOURCE_DIR}/src)

target_link_libraries(test_mixer kmsclib test_memory rtp-test-lib)
include_directories(test_mixer ${kmsc_SOURCE_DIR}/src/core)
include_directories(test_mixer ${kmsc_SOURCE_DIR}/src)

target_link_libraries(test_rtmp kmsclib test_memory)
include_directories(test_rtmp ${kmsc_SOURCE_DIR}/src/core)
include_directories(test_rtmp ${kmsc_SOURCE_DIR}/src)

add_subdirectory(kmsc-test-endpoints)

add_subdirectory(rtp-test-lib)
