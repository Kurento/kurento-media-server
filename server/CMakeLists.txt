aux_source_directory("handlers" HANDLERS)
aux_source_directory("managers" MANAGERS)
aux_source_directory("types" TYPES)

SET(KMSC_SOURCE main.cpp log.cpp media_config_loader.cpp ${HANDLERS}
							${MANAGERS} ${TYPES})
SET_SOURCE_FILES_PROPERTIES(${KMSC_SOURCE}
		PROPERTIES COMPILE_FLAGS
		-DHAVE_NETINET_IN_H -DUSE_BOOST_THREAD -DHAVE_INTTYPES_H)

add_executable(kmsc ${KMSC_SOURCE})
add_dependencies(kmsc kmsiface-project)

add_library(kmsiface STATIC IMPORTED)
set_target_properties(kmsiface PROPERTIES IMPORTED_LOCATION ${KMSIFACE_LIB_DIR}/libkmsiface.a)

add_library(mediaspec STATIC IMPORTED)
set_target_properties(mediaspec PROPERTIES IMPORTED_LOCATION ${KMSIFACE_LIB_DIR}/libmediaspec.a)

target_link_libraries(kmsc kmsiface mediaspec ${THRIFT_LIBRARIES})
target_link_libraries(kmsc ${GSTREAMER_LIBRARIES} ${GLIBMM_LIBRARIES})
target_link_libraries(kmsc ${UUID_LIBRARIES} kmsclib)
include_directories(kmsc ${THRIFT_INCLUDE_DIRS} ${GLIBMM_INCLUDE_DIRS})
include_directories(kmsc ${GSTREAMER_INCLUDE_DIRS})
include_directories(kmsc ${UUID_INCLUDE_DIRS})
include_directories(kmsc ${KMSIFACE_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/server)
include_directories(kmsc ${kmsc_SOURCE_DIR}/src/core)
include_directories(kmsc ${kmsc_SOURCE_DIR}/src)
