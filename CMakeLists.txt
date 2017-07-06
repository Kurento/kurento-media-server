cmake_minimum_required(VERSION 2.8)

project("kurento-media-server")
message(STATUS "Running CMake for project: ${PROJECT_NAME}")

message("If KurentoHelpers is not found, you need to install 'kms-cmake-utils' from the Kurento repository")
find_package(KurentoHelpers REQUIRED)

# Set GCC flags common to all projects
# Note - default flags added by dpkg-buildflags for C and C++:
# -g -O2 -fstack-protector-strong -Wformat -Werror=format-security -Wdate-time -D_FORTIFY_SOURCE=2
include(DpkgBuildFlags)
get_dpkg_buildflags_c(DPKG_BUILDFLAGS_C)
get_dpkg_buildflags_cxx(DPKG_BUILDFLAGS_CXX)
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${DPKG_BUILDFLAGS_C}   -std=c11   -Wall -Werror -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DPKG_BUILDFLAGS_CXX} -std=c++11 -Wall -Werror -pthread")
set(CMAKE_C_FLAGS_DEBUG   "-g -O0") # FIXME Ideal is '-Og' but a bug in GCC prevents this,
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0") # causing "may be used uninitialized" errors: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=58455
set(CMAKE_C_FLAGS_RELEASE   "-DNDEBUG") # CMake adds '-O3' by default,
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG") # which would overwrite our '-O2' from dpkg-buildflags
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # Use "-fPIC" for all targets, including static libs

# FIXME Disable error when macros __TIME__, __DATE__ or __TIMESTAMP__ are encountered
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-Wno-error=date-time" HAS_WARNING_DATE_TIME)
if(HAS_WARNING_DATE_TIME)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=date-time")
endif()

# FIXME Disable error when deprecated functionality is used
# (glibmm library uses C++ dynamic exception specifications)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=deprecated")

# This project includes an input "config.h.cmake"
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -DHAVE_CONFIG_H")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_CONFIG_H")

# Disable deprecated GStreamer APIs
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -DGST_DISABLE_DEPRECATED")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGST_DISABLE_DEPRECATED")

include(KurentoGitHelpers)
get_git_version(PROJECT_VERSION 6.6.2-dev)
install_git_hook(pre-commit ${CMAKE_CURRENT_SOURCE_DIR}/hooks/pre-commit.hook)

message(STATUS "Compiling ${PROJECT_NAME}-${PROJECT_VERSION}")

include(GNUInstallDirs)

set(DISABLE_TESTS FALSE CACHE BOOL "Disable make check")
set(CUSTOM_PREFIX "kurento")
set(INCLUDE_PREFIX "include/${CUSTOM_PREFIX}")
set(LIB_PREFIX "lib")

include(GenericFind)

set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
generic_find(LIBNAME Boost COMPONENTS system filesystem program_options unit_test_framework thread log REQUIRED)

find_package(OpenSSL REQUIRED)

generic_find(LIBNAME KMSCORE VERSION ^6.6.0 REQUIRED)

find_package(PkgConfig)
pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.5)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.5>=1.7.0~0)
pkg_check_modules(GSTREAMER_SDP REQUIRED gstreamer-sdp-1.5>=1.7.0~0)
pkg_check_modules(EVENT REQUIRED libevent>=2.0.16-stable)

check_version_internal(>=2.6.2 sigc++-2.0_VERSION OUTPUT_ERROR)
if(DEFINED OUTPUT_ERROR AND NOT ${OUTPUT_ERROR} EQUAL "")
  set(FIX_SIGC TRUE)
else()
  set(FIX_SIGC FALSE)
endif()

set(TEST_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_subdirectory(server)

enable_testing()

add_subdirectory(test)

install(FILES kurento.conf.json sdp_pattern.txt DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${CUSTOM_PREFIX})

add_subdirectory(scaffold)
