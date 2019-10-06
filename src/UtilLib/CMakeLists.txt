# KaefUtil library
#set(LIB_PROJECT_NAME "${SOLUTION_NAME}")
project(UtilLib VERSION 1.0)

string(TOUPPER ${LIBRARY_NAME} PROJECT_NAME_UC)
configure_file("Version.h.in"   "Version.h")

#set(PROJECT_LIBRARY_DIR "${PROJECT_SOURCE_DIR}/lib")
#project(${LIB_PROJECT_NAME})
#set(PROJECT_LIBRARY_DIR "${PROJECT_SOURCE_DIR}/lib")

include_directories(${Boost_INCLUDE_DIRS})
file(GLOB_RECURSE SOURCE_FILE_LIST *.cpp *.h)

add_library(UtilLib STATIC ${SOURCE_FILE_LIST})
set_property(TARGET UtilLib
             PROPERTY   ARCHIVE_OUTPUT_NAME_DEBUG   ${LIBRARY_NAME}d)
#set_target_properties(${LIB_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${SOLUTION_NAME})

#target_link_libraries(UtilLib ${Boost_LIBRARIES})


install(TARGETS UtilLib
        PUBLIC_HEADER   DESTINATION   ${CMAKE_INSTALL_INCLUDEDIR}/${LIBRARY_NAME})
        
message("Include dir: ${CMAKE_INSTALL_INCLUDEDIR}")