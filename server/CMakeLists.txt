SET(KMSC_SOURCE
  main.cpp
  version.cpp
  version.hpp
  ServerMethods.cpp
  ServerMethods.hpp
  RequestCache.cpp
  RequestCache.hpp
  CacheEntry.cpp
  CacheEntry.hpp)

add_executable(kurento-media-server ${KMSC_SOURCE})

add_dependencies(kurento-media-server signalHandler transport)

target_link_libraries (kurento-media-server
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  transport
  signalHandler
)

set_property (TARGET kurento-media-server
  PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/gen-cpp
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/transport
    ${CMAKE_CURRENT_SOURCE_DIR}/signalHandler
    ${KMS_CORE_INCLUDE_DIRS}
)

install(TARGETS kurento-media-server RUNTIME DESTINATION bin)

add_subdirectory(transport)
add_subdirectory(signalHandler)
