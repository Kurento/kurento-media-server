set (SOURCE_FILES GStreamerModule.cpp GStreamerFilter.cpp GStreamerFilter.hpp)

find_program(THRIFT_EXEC thrift)
set (THRIFT_FILE ${CMAKE_SOURCE_DIR}/kms-interface/src/main/thrift/KmsMediaGStreamerFilterType.thrift)

function(get_base_name thrift_file out_base_name)
        STRING(REGEX REPLACE "[/]+(([^/]+)[/]+)+(.*).thrift\$" "\\3" base_name "${thrift_file}")
        SET(${out_base_name} ${base_name} PARENT_SCOPE)
endfunction(get_base_name)

SET(CUR_GEN_CPP_FILES "")
get_base_name(${THRIFT_FILE} BASE_NAME)
MESSAGE(STATUS "Processing thrift file: ${BASE_NAME}.thrift")

set(GEN_FILES_DIR ${CMAKE_BINARY_DIR}/gen-cpp)

SET(CPP_FILE "${GEN_FILES_DIR}/${BASE_NAME}_types.cpp")
SET(CUR_GEN_CPP_FILES ${CUR_GEN_CPP_FILES} ${CPP_FILE})
SET(CPP_FILE "${GEN_FILES_DIR}/${BASE_NAME}_constants.cpp")
SET(CUR_GEN_CPP_FILES ${CUR_GEN_CPP_FILES} "${CPP_FILE}")

SET(H_FILE "${GEN_FILES_DIR}/${BASE_NAME}_types.h")
SET(GEN_H_FILES ${GEN_H_FILES} ${H_FILE})
SET(H_FILE "${GEN_FILES_DIR}/${BASE_NAME}_constants.h")
SET(GEN_H_FILES ${GEN_H_FILES} "${H_FILE}")

SET(GEN_CPP_FILES ${GEN_CPP_FILES} ${CUR_GEN_CPP_FILES})

FOREACH(CPP_FILE ${CUR_GEN_CPP_FILES})
        ADD_CUSTOM_COMMAND(
                COMMENT         "Generating source from: ${BASE_NAME}.thrift"

                OUTPUT          "${CPP_FILE}"

                DEPENDS         "${THRIFT_FILE}"

                COMMAND         ${THRIFT_EXEC}
                ARGS            --gen cpp -out "${GEN_FILES_DIR}"
                                "${THRIFT_FILE}"
        )
ENDFOREACH(CPP_FILE)

add_library(gstreamer MODULE ${SOURCE_FILES} ${CUR_GEN_CPP_FILES})

set(CFLAGS "-DHAVE_NETINET_IN_H -DUSE_BOOST_THREAD -DHAVE_INTTYPES_H ")

set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_FLAGS ${CFLAGS})

include_directories(gstreamer ${CMAKE_SOURCE_DIR}/server/types ${KMSIFACE_INCLUDE_DIR})
include_directories(gstreamer ${THRIFT_INCLUDE_DIRS} ${GLIBMM_INCLUDE_DIRS})
include_directories(gstreamer ${GSTREAMER_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/server/)

install(TARGETS gstreamer LIBRARY DESTINATION lib/kurento/modules)
