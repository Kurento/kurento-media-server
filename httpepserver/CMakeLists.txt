cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -DHAVE_INTTYPES_H -std=c++0x")

find_package(PkgConfig)

SET(HTTP_EP_SOURCES
  KmsHttpEPServer.cpp
)

SET(HTTP_EP_HEADERS
  KmsHttpEPServer.h
)

add_library(kmshttpep ${HTTP_EP_SOURCES})

set_target_properties(kmshttpep PROPERTIES
  PUBLIC_HEADER "${HTTP_EP_HEADERS}"
)

include_directories(kmshttpep ${GLIB2_INCLUDE_DIRS} ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(kmshttpep ${GLIB2_LIBRARIES} ${GSTREAMER_LIBRARIES})

INSTALL(TARGETS kmshttpep
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include
)