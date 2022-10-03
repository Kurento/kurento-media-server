SET(KMSC_SOURCE
  main.cpp
  version.cpp
  version.hpp
  ServerMethods.cpp
  ServerMethods.hpp
  ResourceManager.cpp
  ResourceManager.hpp
  RequestCache.cpp
  RequestCache.hpp
  CacheEntry.cpp
  CacheEntry.hpp
  logging.cpp
  logging.hpp
  modules.cpp
  modules.hpp
  loadConfig.cpp
  loadConfig.hpp
  ${PROJECT_SOURCE_DIR}/3rdparty/DeathHandler/death_handler.cc
  ${PROJECT_SOURCE_DIR}/3rdparty/DeathHandler/death_handler.h
)

add_definitions(-DBOOST_LOG_DYN_LINK)

add_executable(kurento-media-server ${KMSC_SOURCE})
if(SANITIZERS_ENABLED)
  add_sanitizers(kurento-media-server)
endif()

add_dependencies(kurento-media-server transport)

target_link_libraries (kurento-media-server
  ${Boost_LIBRARIES}
  transport
  dl
)

set_property (TARGET kurento-media-server
  PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_BINARY_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/transport
    ${KmsJsonRpc_INCLUDE_DIRS}
    ${KMSCORE_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/3rdparty/DeathHandler
)

install(TARGETS kurento-media-server RUNTIME DESTINATION bin)

add_subdirectory(transport)
