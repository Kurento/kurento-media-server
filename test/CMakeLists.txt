include(TestHelpers)

set(TEST_VARIABLES
  "TEST_BIN_PATH=${CMAKE_CURRENT_BINARY_DIR}/../server/kurento-media-server"
  "TEST_CONF_PATH=${CMAKE_BINARY_DIR}/config/kurento.conf.json"
  "GST_PLUGIN_PATH=$ENV{GST_PLUGIN_PATH}:${CMAKE_BINARY_DIR}"
  "KURENTO_MODULES_PATH=$ENV{KURENTO_MODULES_PATH}:${CMAKE_BINARY_DIR}"
  "BOOST_TEST_CATCH_SYSTEM_ERRORS=no"
)

add_definitions(-DBOOST_TEST_DYN_LINK)

add_library(base_test EXCLUDE_FROM_ALL BaseTest.cpp)
if(SANITIZERS_ENABLED)
  add_sanitizers(base_test)
endif()

set_property(TARGET base_test
  PROPERTY
    INCLUDE_DIRECTORIES
      ${KMSCORE_INCLUDE_DIRS}
      ${CMAKE_CURRENT_SOURCE_DIR}/../server/transport/websocket
)

target_link_libraries(base_test
  ${Boost_LIBRARIES}
)

if(NOT DEFINED DISABLE_NETWORK_TESTS OR NOT ${DISABLE_NETWORK_TESTS})

add_test_program(test_server_json server_json_test.cpp)
add_dependencies(test_server_json kurento-media-server)
target_link_libraries(test_server_json
  ${KMSCORE_LIBRARIES}
  ${Boost_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  base_test
)
set_property(TARGET test_server_json
  PROPERTY
    INCLUDE_DIRECTORIES
      ${KMSCORE_INCLUDE_DIRS}
      ${CMAKE_CURRENT_SOURCE_DIR}/../server/transport/websocket
)

set(ENABLE_RESOUCES_CHECKS FALSE CACHE BOOL "Enable resouce limits tests")

if(${ENABLE_RESOUCES_CHECKS})
  add_test_program(test_resources_limit resources_limit_test.cpp)
  add_dependencies(test_resources_limit kurento-media-server)
  target_link_libraries(test_resources_limit
    ${KMSCORE_LIBRARIES}
    ${Boost_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    base_test
  )
  set_property(TARGET test_resources_limit
    PROPERTY
      INCLUDE_DIRECTORIES
        ${KMSCORE_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/../server/transport/websocket
  )
endif()

add_test_program(test_server_events server_events_test.cpp)
add_dependencies(test_server_events kurento-media-server)
target_link_libraries(test_server_events
  ${KMSCORE_LIBRARIES}
  ${Boost_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  base_test
)
set_property(TARGET test_server_events
  PROPERTY
    INCLUDE_DIRECTORIES
      ${KMSCORE_INCLUDE_DIRS}
      ${CMAKE_CURRENT_SOURCE_DIR}/../server/transport/websocket
)

add_test_program(test_server_json_session server_json_session.cpp)
add_dependencies(test_server_json_session kurento-media-server)

target_link_libraries(test_server_json_session
  ${KMSCORE_LIBRARIES}
  ${Boost_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  base_test
)
set_property(TARGET test_server_json_session
  PROPERTY
    INCLUDE_DIRECTORIES
      ${KMSCORE_INCLUDE_DIRS}
      ${CMAKE_CURRENT_SOURCE_DIR}/../server/transport/websocket
)

add_test_program(test_server_duplicate_requests server_duplicate_requests.cpp )
add_dependencies(test_server_duplicate_requests kurento-media-server)
target_link_libraries(test_server_duplicate_requests
  ${KMSCORE_LIBRARIES}
  ${Boost_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  base_test
)
set_property(TARGET test_server_duplicate_requests
  PROPERTY INCLUDE_DIRECTORIES
    ${KMSCORE_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../server/transport/websocket
)

add_test_program(test_config_read
  config_read_test.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../server/loadConfig.cpp)
target_link_libraries(test_config_read
  ${Boost_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  ${Boost_LIBRARIES}
  ${KMSCORE_LIBRARIES}
)
set_property(TARGET test_config_read
  PROPERTY INCLUDE_DIRECTORIES
    ${KMSCORE_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../server
)

add_test_program(test_registrar registrar_test.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../server/transport/websocket/WebSocketRegistrar.cpp)
target_link_libraries(test_registrar
  ${Boost_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  ${Boost_LIBRARIES}
  ${KMSCORE_LIBRARIES}
  ${OPENSSL_LIBRARIES}
)
set_property(TARGET test_registrar
  PROPERTY INCLUDE_DIRECTORIES
    ${KMSCORE_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../server/transport/websocket
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

endif(NOT DEFINED DISABLE_NETWORK_TESTS OR NOT ${DISABLE_NETWORK_TESTS})
