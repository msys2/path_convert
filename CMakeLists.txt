cmake_minimum_required(VERSION 2.8)

project(path_conv C)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions(" -ggdb3 -Wall -Wpedantic -std=c99")

set(SRCS
    src/main.cpp
    src/path_conv.cpp
)

add_executable(${CMAKE_PROJECT_NAME} ${SRCS})
target_link_libraries(${CMAKE_PROJECT_NAME} ${LIBRARIES})

