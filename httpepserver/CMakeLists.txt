cmake_minimum_required(VERSION 2.8)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -DHAVE_INTTYPES_H -std=c++0x")

find_package(PkgConfig)
pkg_check_modules(GIO REQUIRED gio-2.0>=2.36.0)
pkg_check_modules(NICE REQUIRED nice>=0.1.3)

# Helpers
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
include(GLibHelpers)

find_package(LibSoup REQUIRED)

SET(HTTP_EP_SOURCES
  KmsHttpEPServer.cpp
  KmsHttpPost.cpp
  KmsHttpLoop.cpp
)

SET(HTTP_EP_HEADERS
  KmsHttpEPServer.h
  KmsHttpPost.h
  KmsHttpLoop.h
)

set(ENUM_HEADERS
  kmshttpendpointaction.h
)

set(GEN_HEADERS
  kms-enumtypes.h
)

add_glib_marshal(HTTP_EP_SOURCES HTTP_EP_HEADERS kms-marshal kms_marshal)
add_glib_enumtypes(HTTP_EP_SOURCES HTTP_EP_HEADERS kms-enumtypes KMS ${ENUM_HEADERS})

add_library(kmshttpep ${HTTP_EP_SOURCES})

set_target_properties(kmshttpep PROPERTIES
  PUBLIC_HEADER "${HTTP_EP_HEADERS}"
)

include_directories(kmshttpep
  ${GLIB2_INCLUDE_DIRS}
  ${GSTREAMER_INCLUDE_DIRS}
  ${LIBSOUP_INCLUDE_DIRS}
  ${UUID_INCLUDE_DIRS}
  ${GIO_INCLUDE_DIRS}
  ${NICE_INCLUDE_DIRS}
)

target_link_libraries(kmshttpep
  ${GLIB2_LIBRARIES}
  ${GSTREAMER_LIBRARIES}
  ${LIBSOUP_LIBRARIES}
  ${UUID_LIBRARIES}
  ${GIO_LIBRARIES}
  ${NICE_LIBRARIES}
)
