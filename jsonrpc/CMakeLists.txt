set (SOURCE_FILES JsonRpcHandler.cpp JsonRpcUtils.cpp)
set (HEADER_FILES JsonRpcHandler.hpp JsonRpcException.hpp JsonSerializer.hpp
      JsonRpcUtils.hpp)

add_library(jsonrpc ${SOURCE_FILES})

set_target_properties(jsonrpc PROPERTIES PUBLIC_HEADER "${HEADER_FILES}")

include_directories(jsonrpc ${JSONCPP_INCLUDE_DIRS})
target_link_libraries(jsonrpc ${JSONCPP_LIBRARIES})

INSTALL(TARGETS jsonrpc
                        ARCHIVE DESTINATION lib
                        PUBLIC_HEADER DESTINATION include)
