set (TRANSPORT_SOURCES
  Processor.hpp
  Transport.hpp
  TransportFactory.cpp
  TransportFactory.hpp
)

add_library (transport ${TRANSPORT_SOURCES})

add_dependencies(transport thriftTransport rabbitMQTransport)

target_link_libraries(transport
  ${GSTREAMER_LIBRARIES}
  ${KMS_CORE_LIBRARIES}
  thriftTransport
  rabbitMQTransport
)

set_property (TARGET transport
  PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/server/signalHandler
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/thrift/
    ${CMAKE_CURRENT_SOURCE_DIR}/rabbitmq/
    ${GSTREAMER_INCLUDE_DIRS}
    ${GLIBMM_INCLUDE_DIRS}
    ${KMS_CORE_INCLUDE_DIRS}
)

add_subdirectory(thrift)
add_subdirectory(rabbitmq)
