set (THRIFT_TRANSPORT_SOURCES
  ThriftEventHandler.cpp
  ThriftEventHandler.hpp
  ThriftTransport.cpp
  ThriftTransport.hpp
  ThriftTransportHandler.cpp
  ThriftTransportHandler.hpp
  ThriftTransportFactory.cpp
  ThriftTransportFactory.hpp
)

set(FLAGS "-DHAVE_NETINET_IN_H ")

set_source_files_properties(${THRIFT_TRANSPORT_SOURCES}
  PROPERTIES
    COMPILE_FLAGS ${FLAGS}
)

add_library (thriftTransport
  ${THRIFT_TRANSPORT_SOURCES}
)

add_dependencies(thriftTransport kmsiface)

target_link_libraries(thriftTransport
  ${GSTREAMER_LIBRARIES}
  -lpthread
  ${THRIFT_LIBRARIES}
  ${KMS_CORE_LIBRARIES}
  ${EVENT_LIBRARIES} #Required by thrift
  kmsiface
)

set_property (TARGET thriftTransport
  PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_BINARY_DIR}/gen-cpp
    ${CMAKE_SOURCE_DIR}/server/transport
    ${GSTREAMER_INCLUDE_DIRS}
    ${THRIFT_INCLUDE_DIRS}
    ${KMS_CORE_INCLUDE_DIRS}
)
