cmake_minimum_required(VERSION 2.8)

project(kmsc-endpoints)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall")

add_subdirectory(src)

add_subdirectory(test)

enable_testing()

add_test(test-endpoints1 test/test-endpoints)
