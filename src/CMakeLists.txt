cmake_minimum_required(VERSION 2.8)

add_library(kmsclib SHARED dummy.c)

add_subdirectory(core)

add_subdirectory(rtp)

link_libraries(kmsclib kmsc-core kmsc-rtp)

install(TARGETS kmsclib LIBRARY DESTINATION ${LIB_PREFIX} PERMISSIONS OWNER_EXECUTE OWNER_READ GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
