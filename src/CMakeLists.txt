cmake_minimum_required(VERSION 2.8)

aux_source_directory("core" CORE)
aux_source_directory("rtp" RTP)

add_library(kmsclib SHARED ${CORE} ${RTP})
add_library(kmsclibst STATIC ${CORE} ${RTP})
SET_TARGET_PROPERTIES(kmsclib kmsclibst PROPERTIES OUTPUT_NAME kmsc)

find_package(PkgConfig)

include_directories(kmsclib ${GLIB2_INCLUDE_DIRS})
target_link_libraries(kmsclib ${GLIB2_LIBRARIES})

include_directories(kmsclib ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(kmsclib ${GSTREAMER_LIBRARIES})

include_directories(kmsclib core rtp)

install(TARGETS kmsclib LIBRARY DESTINATION ${LIB_PREFIX} PERMISSIONS OWNER_EXECUTE OWNER_READ GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install(TARGETS kmsclibst ARCHIVE DESTINATION ${LIB_PREFIX} PERMISSIONS OWNER_EXECUTE OWNER_READ GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

file(GLOB INCLUDES_CORE "core/*.h")

install(FILES ${INCLUDES_CORE} DESTINATION ${INCLUDE_PREFIX} PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

file(GLOB INCLUDES_RTP "rtp/*.h")

install(FILES ${INCLUDES_RTP} DESTINATION ${INCLUDE_PREFIX}/rtp PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
