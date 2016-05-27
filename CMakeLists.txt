cmake_minimum_required(VERSION 2.8)

project ("kurento-media-server")

message ("If KurentoHelpers is not found, then it is necessary to install
          kms-cmake-utils package from the kurento repository")

find_package(KurentoHelpers REQUIRED)

include (KurentoGitHelpers)

get_git_version (PROJECT_VERSION 6.5.1-dev)
install_git_hook (pre-commit ${CMAKE_CURRENT_SOURCE_DIR}/hooks/pre-commit.hook)

message (STATUS "Compiling ${PROJECT_NAME}-${PROJECT_VERSION}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

include(GNUInstallDirs)

set (DISABLE_TESTS FALSE CACHE BOOL "Disable make check")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Werror -std=c++11 -Wno-deprecated-declarations -DGST_DISABLE_DEPRECATED")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Werror -std=c99 -DGST_DISABLE_DEPRECATED")

include(CheckCXXCompilerFlag)

CHECK_CXX_COMPILER_FLAG("-Wno-error=date-time" COMPILER_SUPPORTS_DATETIME_ERROR)
if (COMPILER_SUPPORTS_DATETIME_ERROR)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=date-time")
endif ()

set(CUSTOM_PREFIX "kurento")
set(INCLUDE_PREFIX "include/${CUSTOM_PREFIX}")
set(LIB_PREFIX "lib")

include (GenericFind)

find_package(PkgConfig)
generic_find(LIBNAME Boost COMPONENTS system filesystem program_options unit_test_framework thread log REQUIRED)
find_package(OpenSSL REQUIRED)

generic_find(LIBNAME KMSCORE VERSION ^6.3.0 REQUIRED)
pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.5)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.5>=1.7.0~0)
pkg_check_modules(GSTREAMER_SDP REQUIRED gstreamer-sdp-1.5>=1.7.0~0)
pkg_check_modules(EVENT REQUIRED libevent>=2.0.16-stable)

check_version_internal (>=2.6.2 sigc++-2.0_VERSION OUTPUT_ERROR)
if (DEFINED OUTPUT_ERROR AND NOT ${OUTPUT_ERROR} EQUAL "")
  set (FIX_SIGC TRUE)
else()
  set (FIX_SIGC FALSE)
endif()

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_subdirectory(server)

enable_testing()

add_subdirectory(test)

install(FILES kurento.conf.json sdp_pattern.txt DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${CUSTOM_PREFIX})

add_subdirectory(scaffold)

