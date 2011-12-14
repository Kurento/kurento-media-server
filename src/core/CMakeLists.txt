cmake_minimum_required(VERSION 2.8)

aux_source_directory("." INTERFACES)

add_library(kmsc-core ${INTERFACES})

find_package(PkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(kmsc-core ${GLIB2_INCLUDE_DIRS})
target_link_libraries(kmsc-core ${GLIB2_LIBRARIES})

pkg_check_modules(GSTREAMER REQUIRED gstreamer-0.10>=0.10.35)
include_directories(kmsc-core ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(kmsc-core ${GSTREAMER_LIBRARIES})

file(GLOB INCLUDES_CORE "." "*.h")

INSTALL(FILES ${INCLUDES_CORE} DESTINATION ${INCLUDE_PREFIX} PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
