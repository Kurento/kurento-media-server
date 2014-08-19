set (WEBSOCKET_TRANSPORT
  WebSocketTransport.cpp
  WebSocketTransport.hpp
  WebSocketTransportFactory.cpp
  WebSocketTransportFactory.hpp
)

set(FLAGS "-D_WEBSOCKETPP_CPP11_STL_ ")

set_source_files_properties(${WEBSOCKET_SOURCES}
  PROPERTIES
    COMPILE_FLAGS ${FLAGS}
)

add_library (websocketTransport
  ${WEBSOCKET_TRANSPORT}
)

target_link_libraries(websocketTransport
  ${GSTREAMER_LIBRARIES}
)

set_property (TARGET websocketTransport
  PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/server/transport
    ${CMAKE_SOURCE_DIR}/server/signalHandler
    ${GSTREAMER_INCLUDE_DIRS}
    ${KMS_CORE_INCLUDE_DIRS}
)
