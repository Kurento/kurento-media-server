cmake_minimum_required(VERSION 2.8)

aux_source_directory("." RTP)

add_library(kmsc-rtp ${RTP})

find_package(PkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(kmsc-rtp ${GLIB2_INCLUDE_DIRS})
target_link_libraries(kmsc-rtp ${GLIB2_LIBRARIES})

pkg_check_modules(GSTREAMER REQUIRED gstreamer-0.10>=0.10.35)
include_directories(kmsc-rtp ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(kmsc-rtp ${GSTREAMER_LIBRARIES})

include_directories(kmsc-rtp ${kmsc-endpoints_SOURCE_DIR}/src/core)
