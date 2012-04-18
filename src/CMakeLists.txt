cmake_minimum_required(VERSION 2.8)

aux_source_directory("core" CORE)
aux_source_directory("rtp" RTP)
aux_source_directory("mixer" MIXER)
aux_source_directory("rtmp" RTMP)
aux_source_directory("player" RTMP)

add_library(kmsclib SHARED ${CORE} ${RTP} ${MIXER} ${RTMP} ${PLAYER})
add_library(kmsclibst STATIC ${CORE} ${RTP} ${MIXER} ${RTMP} ${PLAYER})
SET_TARGET_PROPERTIES(kmsclib kmsclibst PROPERTIES OUTPUT_NAME kmsc)

find_package(PkgConfig)

pkg_check_modules(UUID REQUIRED uuid)

include_directories(kmsclib ${UUID_INCLUDE_DIRS})
target_link_libraries(kmsclib ${UUID_LIBRARIES})

include_directories(kmsclib ${GLIB2_INCLUDE_DIRS})
target_link_libraries(kmsclib ${GLIB2_LIBRARIES})

include_directories(kmsclib ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(kmsclib ${GSTREAMER_LIBRARIES})

include_directories(kmsclib ${THRIFTC_INCLUDE_DIRS})
target_link_libraries(kmsclib ${THRIFTC_LIBRARIES})

include_directories(kmsclib core .)

add_library(gmediaspec STATIC IMPORTED)
set_target_properties(gmediaspec PROPERTIES IMPORTED_LOCATION ${KMSIFACE_LIB_DIR}/libgmediaspec.a)

include_directories(kmsclib ${KMSIFACE_INCLUDE_DIR})
target_link_libraries(kmsclib gmediaspec)

install(TARGETS kmsclib LIBRARY DESTINATION ${LIB_PREFIX} PERMISSIONS OWNER_EXECUTE OWNER_READ GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install(TARGETS kmsclibst ARCHIVE DESTINATION ${LIB_PREFIX} PERMISSIONS OWNER_EXECUTE OWNER_READ GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

file(GLOB INCLUDES_CORE "core/*.h")

install(FILES ${INCLUDES_CORE} DESTINATION ${INCLUDE_PREFIX} PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

file(GLOB INCLUDES_RTP "rtp/*.h")

install(FILES ${INCLUDES_RTP} DESTINATION ${INCLUDE_PREFIX}/rtp PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

file(GLOB INCLUDES_MIXER "mixer/*.h")

install(FILES ${INCLUDES_MIXER} DESTINATION ${INCLUDE_PREFIX}/mixer PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

file(GLOB INCLUDES_RTMP "rtmp/*.h")

install(FILES ${INCLUDES_RTMP} DESTINATION ${INCLUDE_PREFIX}/rtmp PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

file(GLOB INCLUDES_PLAYER "player/*.h")

install(FILES ${INCLUDES_PLAYER} DESTINATION ${INCLUDE_PREFIX}/player PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
