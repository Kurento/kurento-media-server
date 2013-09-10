cmake_minimum_required(VERSION 2.8)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -DHAVE_INTTYPES_H -std=c++0x")

find_package(PkgConfig)

# Helpers
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
include(GLibHelpers)
include(GstMacros)

pkg_check_modules(PC_LIBSOUP REQUIRED libsoup-2.4)
find_path(LIBSOUP_INCLUDE_DIRS
    NAMES libsoup/soup.h
    HINTS ${PC_LIBSOUP_INCLUDEDIR}
          ${PC_LIBSOUP_INCLUDE_DIRS}
    PATH_SUFFIXES libsoup-2.4
)

find_library(LIBSOUP_LIBRARIES
    NAMES soup-2.4
    HINTS ${PC_LIBSOUP_LIBDIR}
          ${PC_LIBSOUP_LIBRARY_DIRS}
)

SET(HTTP_EP_SOURCES
  KmsHttpEPServer.cpp
)

SET(HTTP_EP_HEADERS
  KmsHttpEPServer.h
)

set(ENUM_HEADERS
  kmshttpendpointaction.h
)

set(GEN_HEADERS
  kms-enumtypes.h
)

add_glib_marshal(HTTP_EP_SOURCES kms-marshal kms_marshal)
add_glib_enumtypes(HTTP_EP_SOURCES kms-enumtypes KMS ${ENUM_HEADERS})

add_library(kmshttpep ${HTTP_EP_SOURCES})

set_target_properties(kmshttpep PROPERTIES
  PUBLIC_HEADER "${HTTP_EP_HEADERS}"
)

include_directories(kmshttpep
  ${GLIB2_INCLUDE_DIRS}
  ${GSTREAMER_INCLUDE_DIRS}
  ${LIBSOUP_INCLUDE_DIRS}
  ${UUID_INCLUDE_DIRS}
)

target_link_libraries(kmshttpep
  ${GLIB2_LIBRARIES}
  ${GSTREAMER_LIBRARIES}
  ${LIBSOUP_LIBRARIES}
  ${UUID_LIBRARIES}
)

INSTALL(TARGETS kmshttpep
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include
)
