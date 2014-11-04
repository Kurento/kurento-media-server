SET(KMSC_SOURCE
  main.cpp
  version.cpp
  version.hpp
  ServerMethods.cpp
  ServerMethods.hpp
  RequestCache.cpp
  RequestCache.hpp
  CacheEntry.cpp
  CacheEntry.hpp
  logging.cpp
  logging.hpp
  modules.cpp
  modules.hpp
  dtlsfix.cpp
)

add_definitions(-DBOOST_LOG_DYN_LINK)

add_executable(kurento-media-server ${KMSC_SOURCE})

add_dependencies(kurento-media-server signalHandler transport)

target_link_libraries (kurento-media-server
  ${Boost_LIBRARIES}
  ${GNUTLS_LIBRARIES}
  transport
  signalHandler
  pthread
)

set_property (TARGET kurento-media-server
  PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/gen-cpp
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/transport
    ${CMAKE_CURRENT_SOURCE_DIR}/signalHandler
    ${KMSCORE_INCLUDE_DIRS}
    ${GNUTLS_INCLUDE_DIRS}
)

install(TARGETS kurento-media-server RUNTIME DESTINATION bin)

add_subdirectory(transport)
add_subdirectory(signalHandler)
