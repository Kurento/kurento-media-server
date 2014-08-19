add_library (signalHandler
  SignalHandler.cpp
  SignalHandler.hpp
)

target_link_libraries (signalHandler
  ${GLIBMM_LIBRARIES}
  ${GSTREAMER_LIBRARIES}
)

set_property (TARGET signalHandler
  PROPERTY INCLUDE_DIRECTORIES
    ${GLIBMM_INCLUDE_DIRS}
    ${GSTREAMER_INCLUDE_DIRS}
)
