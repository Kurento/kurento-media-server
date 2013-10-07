find_package (Boost COMPONENTS unit_test_framework REQUIRED)

aux_source_directory("${CMAKE_SOURCE_DIR}/server/utils" UTILS)

SET(UTILS_TEST_SOURCE utils_test.cpp ${UTILS})
SET_SOURCE_FILES_PROPERTIES(${UTILS_TEST_SOURCE}
                PROPERTIES COMPILE_FLAGS
                -DHAVE_NETINET_IN_H)

add_executable(utils_test ${UTILS_TEST_SOURCE})
add_dependencies(utils_test kmsiface-project)
add_dependencies(utils_test kurento)

target_link_libraries(utils_test
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                      )

target_link_libraries(utils_test kmsiface ${THRIFT_LIBRARIES})
target_link_libraries(utils_test ${GLIBMM_LIBRARIES})
target_link_libraries(utils_test ${UUID_LIBRARIES})

include_directories(utils_test ${THRIFT_INCLUDE_DIRS})
include_directories(utils_test ${KMSIFACE_INCLUDE_DIR})
include_directories(utils_test ${GLIBMM_INCLUDE_DIRS})
include_directories(utils_test ${UUID_INCLUDE_DIRS})
include_directories(utils_test ${CMAKE_SOURCE_DIR}/server)

add_definitions(-DBOOST_TEST_DYN_LINK)


# Temporally disabled
if (0)
add_executable(server_test server_test_base.cpp server_test.cpp sdp_end_point_test.cpp)
endif ()

set(CFLAGS "-DHAVE_NETINET_IN_H ")
if (DEFINED KMS_TEST)
  message("KMS_TEST set")
  set(CFLAGS ${CFLAGS}"-DKMS_TEST ")
endif ()

set(SERVER_TEST_SOURCE server_test server_test_base.cpp server_test.cpp ${UTILS})
set_source_files_properties(${SERVER_TEST_SOURCE}
                PROPERTIES COMPILE_FLAGS ${CFLAGS})

add_executable(${SERVER_TEST_SOURCE})
add_dependencies(server_test kmsiface-project)
add_dependencies(server_test kurento)

target_link_libraries(server_test
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                      )
target_link_libraries(server_test ${EVENT_LIBRARIES})
target_link_libraries(server_test kmsiface ${THRIFT_LIBRARIES})
target_link_libraries(server_test ${GSTREAMER_LIBRARIES})
target_link_libraries(server_test ${GLIBMM_LIBRARIES})
target_link_libraries(server_test ${UUID_LIBRARIES})

include_directories(server_test ${THRIFT_INCLUDE_DIRS})
include_directories(server_test ${GSTREAMER_INCLUDE_DIRS})
include_directories(server_test ${GLIBMM_INCLUDE_DIRS})
include_directories(server_test ${UUID_INCLUDE_DIRS})
include_directories(server_test ${KMSIFACE_INCLUDE_DIR})
include_directories(server_test ${CMAKE_SOURCE_DIR}/server)

add_definitions(-DBOOST_TEST_DYN_LINK)

# Temporally disabled
if (0)
add_executable(server_memory_test server_test_base.cpp server_memory_test.cpp
                memory.cpp)
add_dependencies(server_memory_test kmsiface-project)
add_dependencies(server_memory_test kurento)

target_link_libraries(server_memory_test
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                      )
target_link_libraries(server_memory_test ${EVENT_LIBRARIES})
target_link_libraries(server_memory_test kmsiface ${THRIFT_LIBRARIES})
target_link_libraries(server_memory_test ${GSTREAMER_LIBRARIES})
target_link_libraries(server_memory_test ${GLIBMM_LIBRARIES})

include_directories(server_memory_test ${THRIFT_INCLUDE_DIRS})
include_directories(server_memory_test ${GSTREAMER_INCLUDE_DIRS})
include_directories(server_memory_test ${GLIBMM_INCLUDE_DIRS})
include_directories(server_memory_test ${KMSIFACE_INCLUDE_DIR})
include_directories(server_memory_test ${CMAKE_SOURCE_DIR}/server)

add_definitions(-DBOOST_TEST_DYN_LINK)


add_executable(http_ep_server_test
  http_ep_server_test.cpp
)

add_dependencies(http_ep_server_test kurento)

target_link_libraries(http_ep_server_test
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                      )

target_link_libraries(http_ep_server_test kmshttpep)
target_link_libraries(http_ep_server_test ${GSTREAMER_LIBRARIES})
target_link_libraries(http_ep_server_test ${GLIBMM_LIBRARIES})
target_link_libraries(http_ep_server_test ${LIBSOUP_LIBRARIES})

include_directories(http_ep_server_test ${CMAKE_SOURCE_DIR}/httpepserver)
include_directories(http_ep_server_test ${GSTREAMER_INCLUDE_DIRS})
include_directories(http_ep_server_test ${GLIBMM_INCLUDE_DIRS})
include_directories(http_ep_server_test ${LIBSOUP_INCLUDE_DIRS})
include_directories(http_ep_server_test ${CMAKE_SOURCE_DIR}/server)

add_definitions(-DBOOST_TEST_DYN_LINK)
endif ()
