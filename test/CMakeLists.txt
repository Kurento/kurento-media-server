add_executable(server_test server_test.cpp)
add_dependencies(server_test kmsiface-project)
add_dependencies(server_test kurento)

find_package (Boost COMPONENTS unit_test_framework REQUIRED)

target_link_libraries(server_test
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                      )
target_link_libraries(server_test ${EVENT_LIBRARIES})
target_link_libraries(server_test kmsiface ${THRIFT_LIBRARIES})
target_link_libraries(server_test ${GSTREAMER_LIBRARIES})
target_link_libraries(server_test ${GLIBMM_LIBRARIES})

include_directories(server_test ${THRIFT_INCLUDE_DIRS})
include_directories(server_test ${GSTREAMER_INCLUDE_DIRS})
include_directories(server_test ${GLIBMM_INCLUDE_DIRS})
include_directories(server_test ${KMSIFACE_INCLUDE_DIR})
include_directories(server_test ${CMAKE_SOURCE_DIR}/server)

add_definitions(-DBOOST_TEST_DYN_LINK -DBOOST_TEST_MAIN)

install(TARGETS server_test RUNTIME DESTINATION usr/bin)
