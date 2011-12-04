cmake_minimum_required(VERSION 2.8)

project(kmsc-endpoints)

find_package(PkgConfig)

add_subdirectory(src)

add_executable(test-endpoints main.c)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(test-endpoints ${GLIB2_INCLUDE_DIRS})
target_link_libraries(test-endpoints ${GLIB2_LIBRARIES})

include_directories(test-endpoints "src")

target_link_libraries(test-endpoints kmsc-endpoints)
