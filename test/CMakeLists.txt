find_package (Boost COMPONENTS unit_test_framework REQUIRED)
find_package(Boost COMPONENTS system filesystem REQUIRED)

add_definitions(-DBOOST_TEST_DYN_LINK)

if (NOT DEFINED DISABLE_NETWORK_TESTS OR NOT ${DISABLE_NETWORK_TESTS})

set (TEST_PROPERTIES
  "GST_PLUGIN_PATH=${CMAKE_BINARY_DIR}/gst-kurento-plugins"
  "MEDIA_SERVER_CONF_FILE=${CMAKE_SOURCE_DIR}/kurento.conf"
  "TESSDATA_PREFIX=${CMAKE_SOURCE_DIR}/gst-kurento-plugins/src/filters/platedetector"
  "SERVER_DIR=${CMAKE_CURRENT_BINARY_DIR}/../server/kurento"
)

add_test_program (test_http_ep_server http_ep_server_test.cpp)
add_dependencies(test_http_ep_server
  kurento
  kmshttpep
  ALL
)
target_link_libraries(test_http_ep_server
  kmshttpep
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  ${GSTREAMER_LIBRARIES}
  ${GLIBMM_LIBRARIES}
  ${LIBSOUP_LIBRARIES}
)
set_property (TARGET test_http_ep_server PROPERTY INCLUDE_DIRECTORIES
  ${CMAKE_SOURCE_DIR}/httpepserver
  ${GSTREAMER_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
  ${LIBSOUP_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/server
)

add_test_program(test_server_json server_json_test.cpp server_test_base.cpp)
add_dependencies(test_server_json
  ALL
  kmsiface
  kurento
)
target_link_libraries(test_server_json
  kmsiface
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  ${EVENT_LIBRARIES}
  ${THRIFT_LIBRARIES}
  ${GSTREAMER_LIBRARIES}
  ${GLIBMM_LIBRARIES}
  ${UUID_LIBRARIES}
  ${GLIB2_LIBRARIES} -lpthread
  ${JSONCPP_LIBRARIES}
)
set_property (TARGET test_server_json PROPERTY INCLUDE_DIRECTORIES
  ${THRIFT_INCLUDE_DIRS}
  ${GSTREAMER_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
  ${UUID_INCLUDE_DIRS}
  ${KMSIFACE_INCLUDE_DIR}
  ${JSONCPP_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/server
)

add_test_program(server_json_session server_test_base.cpp server_json_session.cpp)
add_dependencies(server_json_session
  ALL
  kmsiface
  kurento
)
target_link_libraries(server_json_session
  kmsiface
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  ${EVENT_LIBRARIES}
  ${THRIFT_LIBRARIES}
  ${GSTREAMER_LIBRARIES}
  ${GLIBMM_LIBRARIES}
  ${UUID_LIBRARIES}
  ${GLIB2_LIBRARIES} -lpthread
  ${JSONCPP_LIBRARIES}
)
set_property (TARGET server_json_session  PROPERTY INCLUDE_DIRECTORIES
  ${THRIFT_INCLUDE_DIRS}
  ${GSTREAMER_INCLUDE_DIRS}
  ${GLIBMM_INCLUDE_DIRS}
  ${UUID_INCLUDE_DIRS}
  ${KMSIFACE_INCLUDE_DIR}
  ${JSONCPP_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/server)

endif(NOT DEFINED DISABLE_NETWORK_TESTS OR NOT ${DISABLE_NETWORK_TESTS})
