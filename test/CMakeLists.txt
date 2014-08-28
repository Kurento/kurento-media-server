include (TestHelpers)

add_definitions(-DBOOST_TEST_DYN_LINK -D_WEBSOCKETPP_CPP11_STL_ -pthread)

if (NOT DEFINED DISABLE_NETWORK_TESTS OR NOT ${DISABLE_NETWORK_TESTS})

set (TEST_PROPERTIES
  "MEDIA_SERVER_CONF_FILE=${CMAKE_SOURCE_DIR}/kurento.conf.json"
  "SERVER_DIR=${CMAKE_CURRENT_BINARY_DIR}/../server/kurento-media-server"
  "BOOST_TEST_CATCH_SYSTEM_ERRORS=no"
)

add_test_program(test_server_json server_json_test.cpp BaseTest.cpp)
add_dependencies(test_server_json
  ALL
  kurento-media-server
)
target_link_libraries(test_server_json
  ${KMS_CORE_LIBRARIES}
  ${Boost_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  -pthread
)
set_property (TARGET test_server_json
  PROPERTY
    INCLUDE_DIRECTORIES
      ${KMS_CORE_INCLUDE_DIRS}
      ${CMAKE_SOURCE_DIR}/server/transport
      ${CMAKE_SOURCE_DIR}/server/transport/websocket
      ${CMAKE_SOURCE_DIR}/server
      ${THRIFT_INCLUDE_DIRS}
)

add_test_program(test_server_json_session BaseTest.cpp server_json_session.cpp)
add_dependencies(test_server_json_session
  ALL
  kurento-media-server
)

target_link_libraries(test_server_json_session
  ${KMS_CORE_LIBRARIES}
  ${Boost_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  -pthread
)
set_property (TARGET test_server_json_session
  PROPERTY
    INCLUDE_DIRECTORIES
      ${KMS_CORE_INCLUDE_DIRS}
      ${CMAKE_SOURCE_DIR}/server/transport
      ${CMAKE_SOURCE_DIR}/server/transport/websocket
      ${CMAKE_SOURCE_DIR}/server/
)

add_test_program(test_server_duplicate_requests BaseTest.cpp server_duplicate_requests.cpp )
add_dependencies(test_server_duplicate_requests
  ALL
  kurento-media-server
)
target_link_libraries(test_server_duplicate_requests
  ${KMS_CORE_LIBRARIES}
  ${Boost_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  -pthread
)
set_property (TARGET test_server_duplicate_requests
  PROPERTY INCLUDE_DIRECTORIES
    ${KMS_CORE_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/server/transport
    ${CMAKE_SOURCE_DIR}/server/transport/websocket
    ${CMAKE_SOURCE_DIR}/server/
)

endif(NOT DEFINED DISABLE_NETWORK_TESTS OR NOT ${DISABLE_NETWORK_TESTS})
