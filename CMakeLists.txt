cmake_minimum_required(VERSION 2.8)

project(kurento-media-server)

if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
	execute_process(COMMAND git describe --abbrev=6 --tags --dirty --always
			OUTPUT_VARIABLE KMSC_VERSION
			OUTPUT_STRIP_TRAILING_WHITESPACE)
	string(REGEX REPLACE "^kmsc-(.*)" "\\1" KMSC_VERSION ${KMSC_VERSION})
else(EXISTS "${CMAKE_SOURCE_DIR}/.git")
	set(KMSC_MAJOR_VERSION 2)
	set(KMSC_MINOR_VERSION 0)
	set(KMSC_PATCH_VERSION 0-dev)
	set(KMSC_VERSION
		${KMSC_MAJOR_VERSION}.${KMSC_MINOR_VERSION}.${KMSC_PATCH_VERSION})
endif(EXISTS "${CMAKE_SOURCE_DIR}/.git")

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# TODO: Check if HAVE_INIT_TYPES_H has to be added (not in all systems)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pedantic -Wall -std=c++0x -DGST_DISABLE_DEPRECATED")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -pedantic -Wall -Werror -std=c99 -DGST_DISABLE_DEPRECATED")

set(CUSTOM_PREFIX "kurento")
set(INCLUDE_PREFIX "include/${CUSTOM_PREFIX}")
set(LIB_PREFIX "lib")

find_package(PkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0>=2.30)
#pkg_check_modules(GSTREAMER REQUIRED gstreamer-0.10>=0.10.36)
pkg_check_modules(THRIFT REQUIRED thrift>=0.9.0)
pkg_check_modules(THRIFTC REQUIRED thrift_c_glib>=0.9.0)
pkg_check_modules(GLIBMM REQUIRED glibmm-2.4>=2.30)
pkg_check_modules(NICE REQUIRED nice>=0.1.1)

include(ExternalProject)

SET(KMSIFACE_INSTALL_DIR ${CMAKE_BINARY_DIR}/kmsiface)
SET(KMSIFACE_INCLUDE_DIR ${KMSIFACE_INSTALL_DIR}/include)
SET(KMSIFACE_LIB_DIR ${KMSIFACE_INSTALL_DIR}/lib)

ExternalProject_Add(
	kmsiface-project
	GIT_REPOSITORY ssh://git@repository.kurento.com/kms/kms-interface.git
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${KMSIFACE_INSTALL_DIR}
	UPDATE_COMMAND git checkout develop
)

add_library(kmsiface STATIC IMPORTED)
set_target_properties(kmsiface PROPERTIES IMPORTED_LOCATION ${KMSIFACE_LIB_DIR}/libkmsiface.a)

add_subdirectory(server)

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

SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libglib2.0-0 (>= 2.30), libgstreamer0.10-0 (>= 0.10.36), libthrift (>= 0.8.0), libthrift_c_glib (>= 0.8.0), libglibmm-2.4-1c2a (>= 2.30), libnice10 (>= 0.1.1), libuuid1, gstreamer0.10-plugins-bad, gstreamer0.10-plugins-farsight, gstreamer0.10-plugins-ugly, gstreamer0.10-plugins-good, gstreamer0.10-plugins-cutter, gstreamer0.10-plugins-base, gstreamer0.10-plugins-bad, lsb-base, logrotate")
# Is also possible to set DEBIAN_PACKAGE_BUILDS_DEPENDS

if(NOT DEFINED SYSCONFDIR)
	set(SYSCONFDIR etc)
endif(NOT DEFINED SYSCONFDIR)

install(FILES kurento.conf DESTINATION ${SYSCONFDIR}/${CUSTOM_PREFIX})
install(FILES "scripts/init.d/kurento" DESTINATION ${SYSCONFDIR}/init.d)
install(FILES "scripts/default/kurento" DESTINATION ${SYSCONFDIR}/default)

include(CPack)
