cmake_minimum_required(VERSION 2.8)

project ("kurento-media-server")

message ("If KurentoHelpers is not found, then it is necessary to install
          kms-cmake-utils package from the kurento repository")

find_package(KurentoHelpers REQUIRED)

include (KurentoGitHelpers)

get_git_version (PROJECT_VERSION 4.3.3)
install_git_hook (pre-commit ${CMAKE_CURRENT_SOURCE_DIR}/hooks/pre-commit.hook)

message (STATUS "Compiling ${PROJECT_NAME}-${PROJECT_VERSION}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

include(GNUInstallDirs)

set (PLUGIN_PATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/gst-kurento-plugins:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/kms-dtls-plugins")

add_subdirectory(kms-interface-rom)

set (DISABLE_TESTS FALSE CACHE BOOL "Disable make check")
set (DISABLE_PLUGIN_TESTS TRUE CACHE BOOL "Disable plugins tests")

set (OLD_DISABLE_TESTS ${DISABLE_TESTS})
set (DISABLE_TESTS ${DISABLE_PLUGIN_TESTS})

add_subdirectory(gst-kurento-plugins)

set (DISABLE_TESTS ${OLD_DISABLE_TESTS})

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# TODO: Check if HAVE_INIT_TYPES_H has to be added (not in all systems)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Werror -std=c++0x -DGST_DISABLE_DEPRECATED")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Werror -std=c99 -DGST_DISABLE_DEPRECATED")

if (${PLATE_DETECTOR_CHECK})
  message (STATUS "Plate detector check enabled")
  add_definitions(-DPLATE_DETECTOR_CHECK)
endif ()

set(CUSTOM_PREFIX "kurento")
set(INCLUDE_PREFIX "include/${CUSTOM_PREFIX}")
set(LIB_PREFIX "lib")

find_package(PkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0>=2.38)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0>=1.3.3)
pkg_check_modules(GSTREAMER_SDP REQUIRED gstreamer-sdp-1.0>=1.3.3)
pkg_check_modules(THRIFT REQUIRED thrift-nb=0.9.0)
pkg_check_modules(EVENT REQUIRED libevent>=2.0.16-stable)
pkg_check_modules(GLIBMM REQUIRED glibmm-2.4>=2.37)
pkg_check_modules(UUID REQUIRED uuid>=2.20.0)
pkg_check_modules(GMODULE REQUIRED gmodule-2.0>=2.38)
pkg_check_modules(JSONRPC REQUIRED libjsonrpc>=0.0.6)
pkg_check_modules(SIGCPP REQUIRED sigc++-2.0>=2.0.10)
pkg_check_modules(RABBITMQ REQUIRED librabbitmq>=0.4.1)

include(ExternalProject)

SET(KMSIFACE_INCLUDE_DIR ${CMAKE_BINARY_DIR}/gen-cpp)

add_subdirectory(kms-interface)

add_subdirectory(httpepserver)

add_subdirectory(server)

enable_testing()

add_subdirectory(test)

# Is also possible to set DEBIAN_PACKAGE_BUILDS_DEPENDS
if(NOT DEFINED SYSCONF_INSTALL_DIR)
	set(SYSCONF_INSTALL_DIR etc CACHE PATH
		    "Install directory for system-wide configuration files")
endif(NOT DEFINED SYSCONF_INSTALL_DIR)

install(FILES kurento.conf pattern.sdp DESTINATION ${SYSCONF_INSTALL_DIR}/${CUSTOM_PREFIX})

include(CPack)
