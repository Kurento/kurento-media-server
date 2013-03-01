aux_source_directory("types" TYPES)

SET(KMSC_SOURCE main.cpp log.cpp media_config_loader.cpp version.cpp MediaServerServiceHandler.cpp
						${TYPES})
SET_SOURCE_FILES_PROPERTIES(${KMSC_SOURCE}
		PROPERTIES COMPILE_FLAGS
		-DHAVE_NETINET_IN_H -DUSE_BOOST_THREAD -DHAVE_INTTYPES_H)

add_executable(kurento ${KMSC_SOURCE})
add_dependencies(kurento kmsiface-project)

target_link_libraries(kurento kmsiface ${THRIFT_LIBRARIES} ${EVENT_LIBRARIES})
target_link_libraries(kurento ${GSTREAMER_LIBRARIES} ${GLIBMM_LIBRARIES})
target_link_libraries(kurento ${UUID_LIBRARIES})
target_link_libraries(kurento ${GLIB2_LIBRARIES})
include_directories(kurento ${CMAKE_BINARY_DIR})
include_directories(kurento ${THRIFT_INCLUDE_DIRS} ${GLIBMM_INCLUDE_DIRS})
include_directories(kurento ${GSTREAMER_INCLUDE_DIRS})
include_directories(kurento ${UUID_INCLUDE_DIRS})
include_directories(kurento ${KMSIFACE_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/server)
include_directories(kurento ${CMAKE_SOURCE_DIR}/src/core)
include_directories(kurento ${CMAKE_SOURCE_DIR}/src)

install(TARGETS kurento RUNTIME DESTINATION usr/bin)
