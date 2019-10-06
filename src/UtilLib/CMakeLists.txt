# KaefUtil library
project(UtilLib VERSION 1.0)

string(TOUPPER ${LIBRARY_NAME} PROJECT_NAME_UC)
configure_file("Version.h.in"   "Version.h")

include_directories(${Boost_INCLUDE_DIRS})
file(GLOB_RECURSE SOURCE_FILE_LIST *.cpp *.h)

add_library(UtilLib STATIC ${SOURCE_FILE_LIST})
set_target_properties(UtilLib   PROPERTIES
                      PUBLIC_HEADER               "Version.h;DateTime.h;DateTime_boost.h;DateTimeBase.h"
                      ARCHIVE_OUTPUT_NAME         ${LIBRARY_NAME}
                      ARCHIVE_OUTPUT_NAME_DEBUG   ${LIBRARY_NAME}d)

install(TARGETS UtilLib
        ARCHIVE         DESTINATION   "${CMAKE_INSTALL_PREFIX}/lib"
        PUBLIC_HEADER   DESTINATION   "${CMAKE_INSTALL_INCLUDE}/${LIBRARY_NAME}")
