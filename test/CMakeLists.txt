include (TestHelpers)

add_definitions(-DBOOST_TEST_DYN_LINK -D_WEBSOCKETPP_CPP11_STL_ -pthread)

add_library (base_test EXCLUDE_FROM_ALL BaseTest.cpp)
set_property (TARGET base_test
  PROPERTY
    INCLUDE_DIRECTORIES
      ${KMSCORE_INCLUDE_DIRS}
      ${CMAKE_SOURCE_DIR}/server/transport/websocket
)

target_link_libraries(base_test
  ${Boost_LIBRARIES}
)

if (NOT DEFINED DISABLE_NETWORK_TESTS OR NOT ${DISABLE_NETWORK_TESTS})

set (TEST_PROPERTIES
  "MEDIA_SERVER_CONF_FILE=${CMAKE_SOURCE_DIR}/kurento.conf.json"
  "SERVER_DIR=${CMAKE_CURRENT_BINARY_DIR}/../server/kurento-media-server"
  "BOOST_TEST_CATCH_SYSTEM_ERRORS=no"
)

add_test_program(test_server_json server_json_test.cpp)
add_dependencies(test_server_json
  kurento-media-server
  base_test
)
target_link_libraries(test_server_json
  ${KMSCORE_LIBRARIES}
  ${Boost_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  -pthread
  base_test
)
set_property (TARGET test_server_json
  PROPERTY
    INCLUDE_DIRECTORIES
      ${KMSCORE_INCLUDE_DIRS}
      ${CMAKE_SOURCE_DIR}/server/transport/websocket
)

set (ENABLE_RESOUCES_CHECKS FALSE CACHE BOOL "Enable resouce limits tests")

if (${ENABLE_RESOUCES_CHECKS})
  add_test_program(test_resources_limit resources_limit_test.cpp)
  add_dependencies(test_resources_limit
    kurento-media-server
    base_test
  )
  target_link_libraries(test_resources_limit
    ${KMSCORE_LIBRARIES}
    ${Boost_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    -pthread
    base_test
  )
  set_property (TARGET test_resources_limit
    PROPERTY
      INCLUDE_DIRECTORIES
        ${KMSCORE_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/server/transport/websocket
  )
endif ()

add_test_program(test_server_events server_events_test.cpp)
add_dependencies(test_server_events
  kurento-media-server
  base_test
)
target_link_libraries(test_server_events
  ${KMSCORE_LIBRARIES}
  ${Boost_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  -pthread
  base_test
)
set_property (TARGET test_server_events
  PROPERTY
    INCLUDE_DIRECTORIES
      ${KMSCORE_INCLUDE_DIRS}
      ${CMAKE_SOURCE_DIR}/server/transport/websocket
)

add_test_program(test_server_json_session server_json_session.cpp)
add_dependencies(test_server_json_session
  kurento-media-server
  base_test
)

target_link_libraries(test_server_json_session
  ${KMSCORE_LIBRARIES}
  ${Boost_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  -pthread
  base_test
)
set_property (TARGET test_server_json_session
  PROPERTY
    INCLUDE_DIRECTORIES
      ${KMSCORE_INCLUDE_DIRS}
      ${CMAKE_SOURCE_DIR}/server/transport/websocket
)

add_test_program(test_server_duplicate_requests server_duplicate_requests.cpp )
add_dependencies(test_server_duplicate_requests
  kurento-media-server
  base_test
)
target_link_libraries(test_server_duplicate_requests
  ${KMSCORE_LIBRARIES}
  ${Boost_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  -pthread
  base_test
)
set_property (TARGET test_server_duplicate_requests
  PROPERTY INCLUDE_DIRECTORIES
    ${KMSCORE_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/server/transport/websocket
)

add_test_program (test_config_read
  config_read_test.cpp
  ${CMAKE_SOURCE_DIR}/server/loadConfig.cpp)
target_link_libraries(test_config_read
  ${Boost_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  ${Boost_LIBRARIES}
  ${KMSCORE_LIBRARIES}
  -pthread
)
set_property (TARGET test_config_read
  PROPERTY INCLUDE_DIRECTORIES
    ${KMSCORE_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/server
)

add_test_program (test_registrar registrar_test.cpp ${CMAKE_SOURCE_DIR}/server/transport/websocket/WebSocketRegistrar.cpp)
target_link_libraries(test_registrar
  ${Boost_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  ${Boost_LIBRARIES}
  ${KMSCORE_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  -pthread
)
set_property (TARGET test_registrar
  PROPERTY INCLUDE_DIRECTORIES
    ${KMSCORE_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/server/transport/websocket
)

endif(NOT DEFINED DISABLE_NETWORK_TESTS OR NOT ${DISABLE_NETWORK_TESTS})
