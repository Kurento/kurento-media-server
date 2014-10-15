set (TRANSPORT_SOURCES
  Processor.hpp
  Transport.hpp
  TransportFactory.cpp
  TransportFactory.hpp
)

add_library (transport ${TRANSPORT_SOURCES})

add_dependencies(transport rabbitMQTransport)

target_link_libraries(transport
  ${GSTREAMER_LIBRARIES}
  ${KMSCORE_LIBRARIES}
  rabbitMQTransport
  websocketTransport
)

set_property (TARGET transport
  PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/server/signalHandler
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/rabbitmq/
    ${CMAKE_CURRENT_SOURCE_DIR}/websocket/
    ${GSTREAMER_INCLUDE_DIRS}
    ${GLIBMM_INCLUDE_DIRS}
    ${KMSCORE_INCLUDE_DIRS}
)

add_subdirectory(rabbitmq)

add_subdirectory(websocket)
