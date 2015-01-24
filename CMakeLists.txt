cmake_minimum_required(VERSION 2.8)

project ("kurento-media-server")

message ("If KurentoHelpers is not found, then it is necessary to install
          kms-cmake-utils package from the kurento repository")

find_package(KurentoHelpers REQUIRED)

include (KurentoGitHelpers)

get_git_version (PROJECT_VERSION 5.1.0)
install_git_hook (pre-commit ${CMAKE_CURRENT_SOURCE_DIR}/hooks/pre-commit.hook)

message (STATUS "Compiling ${PROJECT_NAME}-${PROJECT_VERSION}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

include(GNUInstallDirs)

set (DISABLE_TESTS FALSE CACHE BOOL "Disable make check")

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Werror -std=c++11 -DGST_DISABLE_DEPRECATED")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Werror -std=c99 -DGST_DISABLE_DEPRECATED")

set(CUSTOM_PREFIX "kurento")
set(INCLUDE_PREFIX "include/${CUSTOM_PREFIX}")
set(LIB_PREFIX "lib")

include (GenericFind)

find_package(PkgConfig)
generic_find(LIBNAME Boost COMPONENTS system filesystem program_options unit_test_framework REQUIRED)
find_package(OpenSSL REQUIRED)

generic_find(LIBNAME KMSCORE VERSION ^5.1.0 REQUIRED)
generic_find(LIBNAME KmsJsonRpc VERSION ^0.2.0 REQUIRED)
pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0>=1.4.4)
pkg_check_modules(GSTREAMER_SDP REQUIRED gstreamer-sdp-1.0>=1.4.4)
pkg_check_modules(EVENT REQUIRED libevent>=2.0.16-stable)
pkg_check_modules(RABBITMQ REQUIRED librabbitmq>=0.4.1)

pkg_check_modules(GNUTLS REQUIRED gnutls>=3.2.11)

add_subdirectory(server)

enable_testing()

add_subdirectory(test)

if(NOT DEFINED SYSCONF_INSTALL_DIR)
	set(SYSCONF_INSTALL_DIR etc CACHE PATH
		    "Install directory for system-wide configuration files")
endif()

install(FILES kurento.conf.json sdp_pattern.txt DESTINATION ${SYSCONF_INSTALL_DIR}/${CUSTOM_PREFIX})

add_subdirectory(scaffold)

