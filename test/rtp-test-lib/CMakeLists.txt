add_library(rtp-test-lib rtp-test-lib.c)
include_directories(test_rtp_connection ${kmsc-endpoints_SOURCE_DIR}/src/core)
include_directories(test_rtp_connection ${kmsc-endpoints_SOURCE_DIR}/src)

target_link_libraries(rtp-test-lib kmsclib)
