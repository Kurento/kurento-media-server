cmake_minimum_required(VERSION 2.8)

add_library(kmsclib SHARED dummy.c)

add_subdirectory(core)

add_subdirectory(rtp)

link_libraries(kmsclib kmsc-core kmsc-rtp)
