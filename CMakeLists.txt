cmake_minimum_required(VERSION 2.8)

project(kurento-media-server)

if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
	execute_process(COMMAND git submodule init WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
	execute_process(COMMAND git submodule update WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
	execute_process(COMMAND git describe --abbrev=6 --tags --dirty --always --match "kmsc-*"
			OUTPUT_VARIABLE KMSC_VERSION
			OUTPUT_STRIP_TRAILING_WHITESPACE)
	string(REGEX REPLACE "^kmsc-(.*)" "\\1" KMSC_VERSION ${KMSC_VERSION})
	if (NOT EXISTS "${CMAKE_SOURCE_DIR}/.git/hooks/pre-commit")
		execute_process(COMMAND ln -s ../../hooks/pre-commit.hook
			${CMAKE_SOURCE_DIR}/.git/hooks/pre-commit)
	endif(NOT EXISTS "${CMAKE_SOURCE_DIR}/.git/hooks/pre-commit")
else(EXISTS "${CMAKE_SOURCE_DIR}/.git")
	set(KMSC_MAJOR_VERSION 2)
	set(KMSC_MINOR_VERSION 1)
	set(KMSC_PATCH_VERSION 0-dev)
	set(KMSC_VERSION
		${KMSC_MAJOR_VERSION}.${KMSC_MINOR_VERSION}.${KMSC_PATCH_VERSION})
endif(EXISTS "${CMAKE_SOURCE_DIR}/.git")

set (GSTREAMER_PLUGINS_DIR ${CMAKE_BINARY_DIR}/plugins)

execute_process(COMMAND bash -c "export FINAL_INSTALL_DIR=/usr;./autogen.sh --prefix=${GSTREAMER_PLUGINS_DIR}" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/gst-kurento-plugins)

add_custom_target(plugins COMMAND make install WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/gst-kurento-plugins)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# TODO: Check if HAVE_INIT_TYPES_H has to be added (not in all systems)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Werror -std=c++0x -DGST_DISABLE_DEPRECATED")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Werror -std=c99 -DGST_DISABLE_DEPRECATED")

set(CUSTOM_PREFIX "kurento")
set(INCLUDE_PREFIX "include/${CUSTOM_PREFIX}")
set(LIB_PREFIX "lib")

find_package(PkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0>=2.38)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0>=1.2.0)
pkg_check_modules(GSTREAMER_SDP REQUIRED gstreamer-sdp-1.0>=1.2.0)
pkg_check_modules(THRIFT REQUIRED thrift-nb=0.9.0)
pkg_check_modules(EVENT REQUIRED libevent>=2.0.16-stable)
pkg_check_modules(GLIBMM REQUIRED glibmm-2.4>=2.37)
pkg_check_modules(UUID REQUIRED uuid>=2.20.0)

include(ExternalProject)

SET(KMSIFACE_INCLUDE_DIR ${CMAKE_BINARY_DIR}/gen-cpp)

add_subdirectory(kms-interface)

add_subdirectory(httpepserver)

add_subdirectory(server)

add_subdirectory(test)

enable_testing()

add_test(utils_test test/utils_test)
add_test(media_handler_test test/media_handler_test)

add_test(server_test test/server_test)
set_tests_properties(server_test PROPERTIES ENVIRONMENT "MEDIA_SERVER_CONF_FILE=${CMAKE_SOURCE_DIR}/kurento.conf")

add_test(http_ep_server_test test/http_ep_server_test)
set_tests_properties(http_ep_server_test PROPERTIES ENVIRONMENT "MEDIA_SERVER_CONF_FILE=${CMAKE_SOURCE_DIR}/kurento.conf")

# Temporally disabled
if (0)
if (DEFINED MEMORY_TEST)
  message("Memory test active")
  add_test(server_memory_test test/server_memory_test)
  set_tests_properties(server_memory_test PROPERTIES ENVIRONMENT "MEDIA_SERVER_CONF_FILE=${CMAKE_SOURCE_DIR}/kurento.conf")
endif()

endif ()
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})




set(CPACK_GENERATOR "DEB;TGZ;STGZ")
set(CPACK_PACKAGE_CONTACT "kurento@googlegroups.com")

set(CPACK_PACKAGE_NAME "kurento")
set(CPACK_PACKAGE_VENDOR "Kurento")
set(CPACK_PACKAGE_VERSION ${KMSC_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/license.txt)
set(CPACK_PACKAGING_INSTALL_PREFIX "/")

include(CheckTypeSize)
check_type_size("void *" SIZEOF_VOID_P BUILTIN_TYPES_ONLY)

if (${SIZEOF_VOID_P} GREATER 4)
	set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
else(${SIZEOF_VOID_P} GREATER 4)
	set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i686")
endif(${SIZEOF_VOID_P} GREATER 4)

SET(CPACK_DEBIAN_PACKAGE_DEPENDS "gstreamer1.0-plugins-base (>= 1.2.0),")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS} gstreamer1.0-libav,")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS} gstreamer1.0-nice,")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS} gstreamer1.0-plugins-bad (>= 1.2.0),")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS} gstreamer1.0-plugins-good (>= 1.2.0),")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS} gstreamer1.0-plugins-ugly (>= 1.2.0),")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS} libglibmm-2.4-1c2a (>= 2.37),")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS} libboost-system1.53.0,")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS} libboost-filesystem1.53.0,")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS} libboost-chrono1.53.0,")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS} libevent-1.4-2,")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS} libthrift0 (>= 0.9.0),")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS} libthrift0 (<= 0.9.1),")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS} libcv2.4,")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS} libopencv-dev,")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS} libtesseract3,")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS} tesseract-ocr-eng,")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS} libuuid1,")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS} binutils,")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS} libsoup2.4-1 (>= 2.37)")

# Is also possible to set DEBIAN_PACKAGE_BUILDS_DEPENDS

if(NOT DEFINED SYSCONFDIR)
	set(SYSCONFDIR etc)
endif(NOT DEFINED SYSCONFDIR)

install(FILES kurento.conf pattern.sdp DESTINATION ${SYSCONFDIR}/${CUSTOM_PREFIX})
install(PROGRAMS "scripts/init.d/kurento" DESTINATION ${SYSCONFDIR}/init.d)
install(FILES "scripts/default/kurento" DESTINATION ${SYSCONFDIR}/default)
install(DIRECTORY ${GSTREAMER_PLUGINS_DIR}/lib/gst-kurento-plugins USE_SOURCE_PERMISSIONS DESTINATION usr/lib)
install(DIRECTORY ${GSTREAMER_PLUGINS_DIR}/share/gst-kurento-plugins USE_SOURCE_PERMISSIONS DESTINATION usr/share)

include(CPack)
