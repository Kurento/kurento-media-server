cmake_minimum_required(VERSION 2.8)

add_executable(test-endpoints main.c)
add_executable(test_sdp test_sdp.c)

find_package(PkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(test-endpoints ${GLIB2_INCLUDE_DIRS})
target_link_libraries(test-endpoints ${GLIB2_LIBRARIES})

pkg_check_modules(GSTREAMER REQUIRED gstreamer-0.10>=0.10.35)
include_directories(test-endpoints ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(test-endpoints ${GSTREAMER_LIBRARIES})

target_link_libraries(test-endpoints kmsc-test-endpoints)

include_directories(test-endpoints ${kmsc-endpoints_SOURCE_DIR}/src/core)
include_directories(test-endpoints kmsc-test-endpoints)

target_link_libraries(test_sdp kmsc-core)

include_directories(test_sdp ${kmsc-endpoints_SOURCE_DIR}/src/core)

add_subdirectory(kmsc-test-endpoints)
