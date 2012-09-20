cmake_minimum_required(VERSION 2.8)

add_library(test_memory memory.c)

add_executable(test-endpoints main.c)
add_executable(test_sdp test_sdp.c)
add_executable(test_rtp test_rtp.c )
add_executable(test_rtp_connection test_rtp_connection.c)
add_executable(test_rtp_transmission test_rtp_transmission.c)
add_executable(test_mixer test_mixer.c)
add_executable(test_server test_server.cpp)
add_executable(test_rtmp test_rtmp.c)
add_executable(test_ice test_ice.cpp)

find_package(PkgConfig)

include_directories(test-endpoints ${GLIB2_INCLUDE_DIRS})
target_link_libraries(test-endpoints ${GLIB2_LIBRARIES})

include_directories(test-endpoints ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(test-endpoints ${GSTREAMER_LIBRARIES})

target_link_libraries(test-endpoints kmsc-test-endpoints)

include_directories(test-endpoints ${CMAKE_SOURCE_DIR}/src/core)
include_directories(test-endpoints kmsc-test-endpoints)

target_link_libraries(test_sdp kmsclib test_memory)
include_directories(test_sdp ${CMAKE_SOURCE_DIR}/src/core)
include_directories(test_sdp ${KMSIFACE_INCLUDE_DIR})

target_link_libraries(test_rtp kmsclib test_memory rtp-test-lib)
include_directories(test_rtp ${CMAKE_SOURCE_DIR}/src/core)
include_directories(test_rtp ${CMAKE_SOURCE_DIR}/src)
include_directories(test_rtp ${KMSIFACE_INCLUDE_DIR})
include_directories(test_rtp ${THRIFTC_INCLUDE_DIRS})

target_link_libraries(test_rtp_connection kmsclib test_memory rtp-test-lib)
include_directories(test_rtp_connection ${CMAKE_SOURCE_DIR}/src/core)
include_directories(test_rtp_connection ${CMAKE_SOURCE_DIR}/src)
include_directories(test_rtp_connection ${KMSIFACE_INCLUDE_DIR})
include_directories(test_rtp_connection ${THRIFTC_INCLUDE_DIRS})

target_link_libraries(test_rtp_transmission kmsclib test_memory rtp-test-lib)
include_directories(test_rtp_transmission ${CMAKE_SOURCE_DIR}/src/core)
include_directories(test_rtp_transmission ${CMAKE_SOURCE_DIR}/src)
include_directories(test_rtp_transmission ${KMSIFACE_INCLUDE_DIR})
include_directories(test_rtp_transmission ${THRIFTC_INCLUDE_DIRS})

target_link_libraries(test_mixer kmsclib test_memory rtp-test-lib)
include_directories(test_mixer ${CMAKE_SOURCE_DIR}/src/core)
include_directories(test_mixer ${CMAKE_SOURCE_DIR}/src)
include_directories(test_mixer ${KMSIFACE_INCLUDE_DIR})
include_directories(test_mixer ${THRIFTC_INCLUDE_DIRS})

target_link_libraries(test_rtmp kmsclib test_memory)
include_directories(test_rtmp ${CMAKE_SOURCE_DIR}/src/core)
include_directories(test_rtmp ${CMAKE_SOURCE_DIR}/src)
include_directories(test_rtmp ${KMSIFACE_INCLUDE_DIR})
include_directories(test_rtmp ${THRIFTC_INCLUDE_DIRS})

target_link_libraries(test_server kmsiface mediaspec ${THRIFT_LIBRARIES})
include_directories(test_server ${KMSIFACE_INCLUDE_DIR})
include_directories(test_server ${THRIFT_INCLUDE_DIRS})
include_directories(test_server ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(test_server ${GSTREAMER_LIBRARIES})

target_link_libraries(test_ice kmsiface mediaspec ${THRIFT_LIBRARIES})
include_directories(test_ice ${KMSIFACE_INCLUDE_DIR})
include_directories(test_ice ${THRIFT_INCLUDE_DIRS})
include_directories(test_ice ${GSTREAMER_INCLUDE_DIRS})
include_directories(test_ice ${NICE_INCLUDE_DIRS})
target_link_libraries(test_ice ${GSTREAMER_LIBRARIES})
target_link_libraries(test_ice ${NICE_LIBRARIES})

add_subdirectory(kmsc-test-endpoints)

add_subdirectory(rtp-test-lib)
