cmake_minimum_required(VERSION 2.8)

project(kmsc-endpoints)

add_subdirectory(src)

add_executable(test-endpoints main.c)

target_link_libraries(test-endpoints kmsc-endpoints)
