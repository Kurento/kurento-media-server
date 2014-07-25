find_package(Boost COMPONENTS system filesystem REQUIRED)

aux_source_directory("common" COMMON)
aux_source_directory("types" TYPES)
aux_source_directory("utils" UTILS)
aux_source_directory("services" SERVICES)

SET(KMSC_SOURCE main.cpp version.cpp ServerMethods.cpp
              ${COMMON} ${TYPES} ${UTILS} ${SERVICES})

set(CFLAGS "-DHAVE_NETINET_IN_H -DUSE_BOOST_THREAD -DHAVE_INTTYPES_H ")

set_source_files_properties(${KMSC_SOURCE}
  PROPERTIES
    COMPILE_FLAGS ${CFLAGS}
)

add_executable(kurento-media-server ${KMSC_SOURCE})

add_dependencies(kurento-media-server kmsiface)

target_link_libraries (kurento-media-server
  kmsiface
  ${KMS_CORE_LIBRARIES}
  ${THRIFT_LIBRARIES}
  ${EVENT_LIBRARIES} #Required by thrift
  -lpthread
  ${RABBITMQ_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)

set_property (TARGET kurento-media-server
  PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/gen-cpp
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/services
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${KMS_CORE_INCLUDE_DIRS}
    ${THRIFT_INCLUDE_DIRS}
    ${RABBITMQ_INCLUDE_DIRS}
)

# target_link_libraries(kurento-media-server kmshttpep)
# target_link_libraries(kurento-media-server kmsiface ${THRIFT_LIBRARIES} ${EVENT_LIBRARIES})
# target_link_libraries(kurento-media-server ${GSTREAMER_LIBRARIES} ${GLIBMM_LIBRARIES})
# target_link_libraries(kurento-media-server ${GSTREAMER_SDP_LIBRARIES} )
# target_link_libraries(kurento-media-server ${UUID_LIBRARIES})
# target_link_libraries(kurento-media-server ${GLIB2_LIBRARIES} -lpthread)
# target_link_libraries(kurento-media-server ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
# target_link_libraries(kurento-media-server ${SIGCPP_LIBRARIES})
# target_link_libraries(kurento-media-server ${GMODULE_LIBRARIES})
# target_link_libraries(kurento-media-server ${JSONRPC_LIBRARIES})
# target_link_libraries(kurento-media-server ${RABBITMQ_LIBRARIES})
# include_directories(kurento-media-server ${CMAKE_SOURCE_DIR}/httpepserver)
# include_directories(kurento-media-server ${CMAKE_BINARY_DIR})
# include_directories(kurento-media-server ${THRIFT_INCLUDE_DIRS} ${GLIBMM_INCLUDE_DIRS})
# include_directories(kurento-media-server ${GSTREAMER_INCLUDE_DIRS})
# include_directories(kurento-media-server ${UUID_INCLUDE_DIRS})
# include_directories(kurento-media-server ${KMSIFACE_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/server)
# include_directories(kurento-media-server ${GMODULE_INCLUDE_DIRS})
# include_directories(kurento-media-server ${JSONRPC_INCLUDE_DIRS})
# include_directories(kurento-media-server ${SIGCPP_INCLUDE_DIRS})
# include_directories(kurento-media-server ${CMAKE_CURRENT_SOURCE_DIR}/utils)
# include_directories(kurento-media-server ${CMAKE_CURRENT_SOURCE_DIR}/common)
# include_directories(kurento-media-server ${CMAKE_SOURCE_DIR}/jsonrpc)
# include_directories(kurento-media-server ${CMAKE_SOURCE_DIR}/server/types)
# include_directories(kurento-media-server ${CMAKE_SOURCE_DIR}/server/services)
# include_directories(kurento-media-server ${CMAKE_BINARY_DIR}/rom-cpp)
# include_directories(kurento-media-server ${RABBITMQ_INCLUDE_DIRS})

install(TARGETS kurento-media-server RUNTIME DESTINATION bin)
