add_library(example MODULE example.cpp)

include_directories(example ${CMAKE_SOURCE_DIR}/server/types ${KMSIFACE_INCLUDE_DIR})
include_directories(example ${THRIFT_INCLUDE_DIRS} ${GLIBMM_INCLUDE_DIRS})
include_directories(example ${GSTREAMER_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/server/)

install(TARGETS example LIBRARY DESTINATION lib/kurento/modules)

add_subdirectory(player)
add_subdirectory(recorder)
add_subdirectory(webrtc)
add_subdirectory(chroma)
add_subdirectory(rtp)
add_subdirectory(zbar)
add_subdirectory(jackvader)
add_subdirectory(pointerdetector)
add_subdirectory(pointerdetector2)
add_subdirectory(platedetector)
add_subdirectory(faceoverlay)
