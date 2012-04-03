#Add here submodule
include(ExternalProject)

SET(KMSIFACE_INSTALL_DIR ${CMAKE_BINARY_DIR}/kmsiface)
SET(KMSIFACE_INCLUDE_DIR ${KMSIFACE_INSTALL_DIR}/include)
SET(KMSIFACE_LIB_DIR ${KMSIFACE_INSTALL_DIR}/lib)

ExternalProject_Add(
	kmsiface-project
	GIT_REPOSITORY ssh://git@repository.kurento.com/kms/kms-interface.git
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${KMSIFACE_INSTALL_DIR}
	UPDATE_COMMAND git checkout master
)

aux_source_directory("handlers" HANDLERS)
aux_source_directory("managers" MANAGERS)

SET(KMSC_SOURCE main.cpp ${HANDLERS} ${MANAGERS})
SET_SOURCE_FILES_PROPERTIES(${KMSC_SOURCE}
		PROPERTIES COMPILE_FLAGS
		-DHAVE_NETINET_IN_H -DUSE_BOOST_THREAD -DHAVE_INTTYPES_H)

add_executable(kmsc ${KMSC_SOURCE})
add_dependencies(kmsc kmsiface-project)

add_library(kmsiface STATIC IMPORTED)
set_target_properties(kmsiface PROPERTIES IMPORTED_LOCATION ${KMSIFACE_LIB_DIR}/libkmsiface.a)

add_library(mediaspec STATIC IMPORTED)
set_target_properties(mediaspec PROPERTIES IMPORTED_LOCATION ${KMSIFACE_LIB_DIR}/libmediaspec.a)

target_link_libraries(kmsc kmsiface mediaspec ${THRIFT_LIBRARIES} boost_thread)
include_directories(kmsc ${THRIFT_INCLUDE_DIRS} ${KMSIFACE_INCLUDE_DIR})
