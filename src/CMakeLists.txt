cmake_minimum_required(VERSION 2.8)

aux_source_directory("." INTERFACES)

add_library(kmsc-endpoints ${INTERFACES})

find_package(PkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(kmsc-endpoints ${GLIB2_INCLUDE_DIRS})
target_link_libraries(kmsc-endpoints ${GLIB2_LIBRARIES})

pkg_check_modules(GSTREAMER REQUIRED gstreamer-0.10>=0.10.35)
include_directories(kmsc-endpoints ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(kmsc-endpoints ${GSTREAMER_LIBRARIES})
