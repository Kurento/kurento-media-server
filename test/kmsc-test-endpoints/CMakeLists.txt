cmake_minimum_required(VERSION 2.8)

aux_source_directory("." TEST_ENDPOINTS)

add_library(kmsc-test-endpoints ${TEST_ENDPOINTS})

find_package(PkgConfig)

include_directories(kmsc-test-endpoints ${GLIB2_INCLUDE_DIRS})
target_link_libraries(kmsc-test-endpoints ${GLIB2_LIBRARIES})

include_directories(kmsc-test-endpoints ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(kmsc-test-endpoints ${GSTREAMER_LIBRARIES})

target_link_libraries(kmsc-test-endpoints kmsclib)

include_directories(test-endpoints ${kmsc-endpoints_SOURCE_DIR}/src/core)
