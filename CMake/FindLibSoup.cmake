# - Try to find LibSoup
# Once done this will define
#
#  LIBSOUP_FOUND - system has LibSoup
#  LIBSOUP_INCLUDE_DIRS - the LibSoup include directory
#  LIBSOUP_LIBRARIES - the libraries needed to use LibSoup

find_package(PkgConfig)
pkg_check_modules(PC_LIBSOUP REQUIRED libsoup-2.4>=2.37)

find_path(LIBSOUP_INCLUDE_DIRS
    NAMES libsoup/soup.h
    HINTS ${PC_LIBSOUP_INCLUDEDIR}
          ${PC_LIBSOUP_INCLUDE_DIRS}
    PATH_SUFFIXES libsoup-2.4
)

find_library(LIBSOUP_LIBRARIES
    NAMES soup-2.4
    HINTS ${PC_LIBSOUP_LIBDIR}
          ${PC_LIBSOUP_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBSOUP_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LIBSOUP REQUIRED_VARS LIBSOUP_INCLUDE_DIRS LIBSOUP_LIBRARIES
                                          VERSION_VAR PC_LIBSOUP_VERSION)

mark_as_advanced(LIBSOUP_INCLUDE_DIRS LIBSOUP_LIBRARIES)


