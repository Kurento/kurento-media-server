set (RABBIT_MQ_TRANSPORT_SOURCES
  RabbitMQTransport.cpp
  RabbitMQTransport.hpp
  RabbitMQPipeline.cpp
  RabbitMQPipeline.hpp
  RabbitMQListener.cpp
  RabbitMQListener.hpp
  RabbitMQEventHandler.cpp
  RabbitMQEventHandler.hpp
  RabbitMQConnection.cpp
  RabbitMQConnection.hpp
  RabbitMQTransportFactory.cpp
  RabbitMQTransportFactory.hpp
  RabbitMQReconnectStrategy.hpp
  ExponentialBackoffStrategy.cpp
  ExponentialBackoffStrategy.hpp
)

add_library (rabbitMQTransport
  ${RABBIT_MQ_TRANSPORT_SOURCES}
)

add_dependencies(rabbitMQTransport signalHandler)

target_link_libraries(rabbitMQTransport
  ${GSTREAMER_LIBRARIES}
  ${RABBITMQ_LIBRARIES}
  ${EVENT_LIBRARIES}
  signalHandler
)

set_property (TARGET rabbitMQTransport
  PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/server/transport
    ${CMAKE_SOURCE_DIR}/server/signalHandler
    ${GSTREAMER_INCLUDE_DIRS}
    ${KMSCORE_INCLUDE_DIRS}
    ${RABBITMQ_INCLUDE_DIRS}
    ${EVENT_INCLUDE_DIRS}
)
