# FindMraa.cmake
#
# Finds the rapidjson library
#
# This will define the following variables
#
#    Mraa_FOUND
#    Mraa_INCLUDE_DIRS
#    Mraa_LIBRARIES
#
# and the following imported targets
#
#     mraa::mraa
#
# Author: Andreas Ziegler - a.ziegler@mt-robot.com

find_package(PkgConfig)
pkg_check_modules(MRAA mraa)

find_path(Mraa_INCLUDE_DIR
    NAMES mraa.h
    PATHS ${PC_Mraa_INCLUDE_DIRS} /usr/include 
)

find_library(Mraa_LIBRARY
  NAMES libmraa.so
  PATHS ${PC_Mraa_LIBRARY_DIRS} /usr/lib
)

set(Mraa_VERSION ${Mraa_VERSION})

mark_as_advanced(Mraa_FOUND Mraa_INCLUDE_DIR Mraa_VERSION Mraa_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Mraa
    REQUIRED_VARS Mraa_INCLUDE_DIR
    REQUIRED_VARS Mraa_LIBRARY
    VERSION_VAR Mraa_VERSION
)

if(Mraa_FOUND)
  set(Mraa_INCLUDE_DIRS ${Mraa_INCLUDE_DIR})
  set(Mraa_LIBRARIES ${Mraa_LIBRARY})
endif()

if(Mraa_FOUND AND NOT TARGET mraa::mraa)
    add_library(mraa::mraa UNKNOWN IMPORTED)
    set_target_properties(mraa::mraa PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${Mraa_INCLUDE_DIRS}"
        IMPORTED_LOCATION "${Mraa_LIBRARIES}"
    )
endif()