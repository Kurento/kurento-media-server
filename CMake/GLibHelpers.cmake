cmake_minimum_required(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

find_program(GLIB_MKENUMS glib-mkenums)
find_program(GLIB_GENMARSHAL glib-genmarshal)

macro(add_glib_marshal outfiles name prefix)
  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name}.h"
    COMMAND ${GLIB_GENMARSHAL} --header "--prefix=${prefix}"
            "${CMAKE_CURRENT_SOURCE_DIR}/${name}.list"
            > "${CMAKE_CURRENT_BINARY_DIR}/${name}.h"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${name}.list"
  )
  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name}.c"
#    COMMAND echo "\\#include \\\"${otherinclude}\\\"" > "${CMAKE_CURRENT_BINARY_DIR}/${name}.c"
    COMMAND echo "\\#include \\\"glib-object.h\\\"" >> "${CMAKE_CURRENT_BINARY_DIR}/${name}.c"
    COMMAND echo "\\#include \\\"${name}.h\\\"" >> "${CMAKE_CURRENT_BINARY_DIR}/${name}.c"
    COMMAND ${GLIB_GENMARSHAL} --body "--prefix=${prefix}"
            "${CMAKE_CURRENT_SOURCE_DIR}/${name}.list"
            >> "${CMAKE_CURRENT_BINARY_DIR}/${name}.c"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${name}.list"
            "${CMAKE_CURRENT_BINARY_DIR}/${name}.h"
  )
  list(APPEND ${outfiles} "${CMAKE_CURRENT_BINARY_DIR}/${name}.c")
endmacro(add_glib_marshal)

macro(add_glib_enumtypes_t outfiles name htemplate ctemplate)
  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name}.h"
    COMMAND ${GLIB_MKENUMS}
            --template "${htemplate}"
            ${ARGN} > "${CMAKE_CURRENT_BINARY_DIR}/${name}.h"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${ARGN} "${htemplate}"
  )
  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name}.c"
    COMMAND ${GLIB_MKENUMS}
            --template "${ctemplate}"
            ${ARGN} > "${CMAKE_CURRENT_BINARY_DIR}/${name}.c"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${ARGN} ${ctemplate}
            "${CMAKE_CURRENT_BINARY_DIR}/${name}.h"
  )
  list(APPEND ${outfiles} "${CMAKE_CURRENT_BINARY_DIR}/${name}.c")
endmacro(add_glib_enumtypes_t)

macro(add_glib_enumtypes outfiles name includeguard)
  set(htemplate "${CMAKE_CURRENT_BINARY_DIR}/${name}.h.template")
  set(ctemplate "${CMAKE_CURRENT_BINARY_DIR}/${name}.c.template")

  # Write the .h template
  add_custom_command(
    OUTPUT ${htemplate} ${ctemplate}
    COMMAND ${CMAKE_COMMAND}
        "-Dctemplate=${ctemplate}"
        "-Dhtemplate=${htemplate}"
        "-Dname=${name}"
        "-Dincludeguard=${includeguard}"
        "\"-Dheaders=${ARGN}\""
        -P "${CMAKE_SOURCE_DIR}/CMake/MakeGLibEnumTemplates.cmake"
    DEPENDS "${CMAKE_SOURCE_DIR}/CMake/MakeGLibEnumTemplates.cmake" ${headers}
  )

  add_glib_enumtypes_t(${outfiles} ${name} ${htemplate} ${ctemplate} ${ARGN})
endmacro(add_glib_enumtypes)
