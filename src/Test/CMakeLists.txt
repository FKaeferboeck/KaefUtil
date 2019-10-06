# KaefUtil unit tests

set(TEST_PROJECT_NAME "${PROJECT_NAME}_Test")
project(${TEST_PROJECT_NAME})

include_directories("${PROJECT_INCL_DIR}" ${Boost_INCLUDE_DIRS})
file(GLOB SOURCE_FILE_LIST *.cpp *.h)

add_executable(${TEST_PROJECT_NAME} ${SOURCE_FILE_LIST})

target_link_libraries(${TEST_PROJECT_NAME} ${Boost_LIBRARIES})