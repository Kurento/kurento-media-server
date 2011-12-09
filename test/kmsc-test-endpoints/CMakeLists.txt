cmake_minimum_required(VERSION 2.8)

aux_source_directory("." TEST_ENDPOINTS)

add_library(kmsc-test-endpoints ${TEST_ENDPOINTS})

find_package(PkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(kmsc-test-endpoints ${GLIB2_INCLUDE_DIRS})
target_link_libraries(kmsc-test-endpoints ${GLIB2_LIBRARIES})

pkg_check_modules(GSTREAMER REQUIRED gstreamer-0.10>=0.10.35)
include_directories(kmsc-test-endpoints ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(kmsc-test-endpoints ${GSTREAMER_LIBRARIES})

target_link_libraries(kmsc-test-endpoints kmsc-core)

include_directories(test-endpoints ${kmsc-endpoints_SOURCE_DIR}/src/core)
