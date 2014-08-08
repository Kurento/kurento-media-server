include (TestHelpers)

add_definitions(-DBOOST_TEST_DYN_LINK)

if (NOT DEFINED DISABLE_NETWORK_TESTS OR NOT ${DISABLE_NETWORK_TESTS})

set (TEST_PROPERTIES
  "MEDIA_SERVER_CONF_FILE=${CMAKE_SOURCE_DIR}/kurento.conf.json"
  "SERVER_DIR=${CMAKE_CURRENT_BINARY_DIR}/../server/kurento-media-server"
  "BOOST_TEST_CATCH_SYSTEM_ERRORS=no"
)

add_test_program(test_server_json server_json_test.cpp server_test_base.cpp)
add_dependencies(test_server_json
  ALL
  kmsiface
  kurento-media-server
)
target_link_libraries(test_server_json
  kmsiface
  ${KMS_CORE_LIBRARIES}
  ${THRIFT_LIBRARIES}
  ${EVENT_LIBRARIES}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)
set_property (TARGET test_server_json
  PROPERTY
    INCLUDE_DIRECTORIES
      ${KMS_CORE_INCLUDE_DIRS}
      ${CMAKE_BINARY_DIR}/gen-cpp
      ${CMAKE_SOURCE_DIR}/server/services
      ${CMAKE_SOURCE_DIR}/server
      ${THRIFT_INCLUDE_DIRS}
)

add_test_program(test_server_json_session server_test_base.cpp server_json_session.cpp)
add_dependencies(test_server_json_session
  ALL
  kmsiface
  kurento-media-server
)

target_link_libraries(test_server_json_session
  kmsiface
  ${KMS_CORE_LIBRARIES}
  ${THRIFT_LIBRARIES}
  ${EVENT_LIBRARIES}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)
set_property (TARGET test_server_json_session
  PROPERTY
    INCLUDE_DIRECTORIES
      ${KMS_CORE_INCLUDE_DIRS}
      ${THRIFT_INCLUDE_DIRS}
      ${CMAKE_BINARY_DIR}/gen-cpp
      ${CMAKE_SOURCE_DIR}/server/services
      ${CMAKE_SOURCE_DIR}/server/
)

endif(NOT DEFINED DISABLE_NETWORK_TESTS OR NOT ${DISABLE_NETWORK_TESTS})
