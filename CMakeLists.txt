cmake_minimum_required(VERSION 2.8)

project(kmsc-project)

if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
	execute_process(COMMAND git describe --abbrev=6 --tags --dirty --always
			OUTPUT_VARIABLE KMSC_VERSION
			OUTPUT_STRIP_TRAILING_WHITESPACE)
	string(REGEX REPLACE "^kmsc-(.*)" "\\1" KMSC_VERSION ${KMSC_VERSION})
else(EXISTS "${CMAKE_SOURCE_DIR}/.git")
	set(KMSC_MAJOR_VERSION 1)
	set(KMSC_MINOR_VERSION 0)
	set(KMSC_PATCH_VERSION 1)
	set(KMSC_VERSION
		${KMSC_MAJOR_VERSION}.${KMSC_MINOR_VERSION}.${KMSC_PATCH_VERSION})
endif(EXISTS "${CMAKE_SOURCE_DIR}/.git")

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# TODO: Check if HAVE_INIT_TYPES_H has to be added (not in all systems)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -std=c++0x")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Werror -std=c99")

set(CUSTOM_PREFIX "kmsc")
set(INCLUDE_PREFIX "include/${CUSTOM_PREFIX}")
set(LIB_PREFIX "lib")

find_package(PkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0>=2.30)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-0.10>=0.10.35)
pkg_check_modules(THRIFT REQUIRED thrift>=0.8.0)
pkg_check_modules(THRIFTC REQUIRED thrift_c_glib>=0.8.0)
pkg_check_modules(GLIBMM REQUIRED glibmm-2.4>=2.30)

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

add_library(kmsiface STATIC IMPORTED)
set_target_properties(kmsiface PROPERTIES IMPORTED_LOCATION ${KMSIFACE_LIB_DIR}/libkmsiface.a)

add_library(mediaspec STATIC IMPORTED)
set_target_properties(mediaspec PROPERTIES IMPORTED_LOCATION ${KMSIFACE_LIB_DIR}/libmediaspec.a)

add_subdirectory(src)
add_subdirectory(server)

add_subdirectory(test)

enable_testing()

add_test(test-endpoints1 test/test-endpoints)
add_test(test_sdp test/test_sdp)
add_test(test_rtp test/test_rtp)
add_test(test_rtp_connection test/test_rtp_connection)
add_test(test_rtp_transmission test/test_rtp_transmission)
add_test(test_mixer test/test_mixer)
add_test(test_rtmp test/test_rtmp)
add_test(test_server test/test_server)

set(CPACK_GENERATOR "DEB;TGZ;STGZ")
set(CPACK_PACKAGE_CONTACT "kurento@googlegroups.com")

set(CPACK_PACKAGE_NAME "kmsc")
set(CPACK_PACKAGE_VENDOR "Kurento")
set(CPACK_PACKAGE_VERSION ${KMSC_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/license.txt)

if(NOT DEFINED SYSCONFDIR)
	set(SYSCONFDIR etc)
endif(NOT DEFINED SYSCONFDIR)

install(FILES kms.conf DESTINATION ${SYSCONFDIR})

include(CPack)
