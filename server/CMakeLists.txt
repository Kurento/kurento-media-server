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

install(TARGETS kurento-media-server RUNTIME DESTINATION bin)
