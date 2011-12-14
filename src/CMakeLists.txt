cmake_minimum_required(VERSION 2.8)

add_subdirectory(core)

add_subdirectory(rtp)

aux_source_directory("core" CORE)
aux_source_directory("rtp" RTP)

add_library(kmsclib SHARED ${CORE} ${RTP})

find_package(PkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(kmsclib ${GLIB2_INCLUDE_DIRS})
target_link_libraries(kmsclib ${GLIB2_LIBRARIES})

pkg_check_modules(GSTREAMER REQUIRED gstreamer-0.10>=0.10.35)
include_directories(kmsclib ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(kmsclib ${GSTREAMER_LIBRARIES})

include_directories(kmsclib core rtp)

install(TARGETS kmsclib LIBRARY DESTINATION ${LIB_PREFIX} PERMISSIONS OWNER_EXECUTE OWNER_READ GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
