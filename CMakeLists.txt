cmake_minimum_required(VERSION 2.8)

project("kurento-media-server")
message(STATUS "CMake project: ${PROJECT_NAME}")

# Test configuration
set(GENERATE_TESTS FALSE CACHE BOOL "Always build tests: add `make check_build` to normal `make` calls")
set(DISABLE_TESTS FALSE CACHE BOOL "Enable running `make check` during the building process")
set(VALGRIND_NUM_CALLERS 20 CACHE STRING "Valgrind option: maximum number of entries shown in stack traces")

message("If KurentoHelpers is not found, you need to install 'kms-cmake-utils' from the Kurento repository")
find_package(KurentoHelpers REQUIRED)

include(KurentoGitHelpers)
install_git_hook(pre-commit ${CMAKE_CURRENT_SOURCE_DIR}/hooks/pre-commit.hook)
get_git_version(PROJECT_VERSION 6.16.1-dev)
message(STATUS "Project version: ${PROJECT_NAME}-${PROJECT_VERSION}")

# Compiler flags
include(CommonBuildFlags)
common_buildflags_set()
#common_buildflags_print()

# Development: Add here exceptions to the "Warnings are Errors" rule.
# Also, DOCUMENT WHY and always remove them as soon as the problem is fixed.
# For example:
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error=unused-function")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error=unused-variable")

# FIXME Disable error from deprecated dynamic exception specifications in glibmm
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=deprecated")

# FIXME Disable error when macros __TIME__, __DATE__ or __TIMESTAMP__ are encountered
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-Wno-error=date-time" HAS_WARNING_DATE_TIME)
if(HAS_WARNING_DATE_TIME)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=date-time")
endif()

# FIXME Disable deprecated GStreamer APIs
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -DGST_DISABLE_DEPRECATED")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGST_DISABLE_DEPRECATED")

include(GenericFind)
generic_find(LIBNAME KMSCORE VERSION ^6.16.1-dev REQUIRED)

# Generate file "config.h"
check_version_internal(>=2.6.2 sigc++-2.0_VERSION OUTPUT_ERROR)
if(DEFINED OUTPUT_ERROR AND NOT ${OUTPUT_ERROR} EQUAL "")
  set(FIX_SIGC TRUE)
else()
  set(FIX_SIGC FALSE)
endif()
set(TEST_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -DHAVE_CONFIG_H")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_CONFIG_H")

set(CUSTOM_PREFIX "kurento")
set(INCLUDE_PREFIX "include/${CUSTOM_PREFIX}")
set(LIB_PREFIX "lib")

set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
generic_find(LIBNAME Boost COMPONENTS system filesystem program_options unit_test_framework thread log REQUIRED)

find_package(OpenSSL REQUIRED)

find_package(PkgConfig)
pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.5)
pkg_check_modules(GSTREAMER_SDP REQUIRED gstreamer-sdp-1.5)
pkg_check_modules(EVENT REQUIRED libevent>=2.0.16)

add_subdirectory(server)

enable_testing()

add_subdirectory(test)


include(GNUInstallDirs)
install(FILES kurento.conf.json sdp_pattern.txt DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${CUSTOM_PREFIX})

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/config)
get_filename_component(CONFIG_FILE kurento.conf.json ABSOLUTE)
execute_process(COMMAND ln -fs ${CONFIG_FILE} ${CMAKE_BINARY_DIR}/config/)


add_subdirectory(scaffold)
