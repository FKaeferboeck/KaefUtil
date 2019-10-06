cmake_minimum_required(VERSION 3.12)

set(LIBRARY_NAME KaefUtil CACHE STRING "Name of project, library & C++ namespace")
project(${LIBRARY_NAME})

string(TOUPPER ${LIBRARY_NAME} LIBRARY_NAME_UC)

#set(SOLUTION_NAME ${LIBRARY_NAME})
#set(PROJECT_INCL_DIR "${PROJECT_SOURCE_DIR}/incl")
#set(PROJECT_LIBRARY_DIR "${PROJECT_SOURCE_DIR}/lib")

set(CMAKE_CXX_STANDARD 14)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.50 REQUIRED COMPONENTS date_time)

add_subdirectory(src/UtilLib)
#add_subdirectory(src/Test)
