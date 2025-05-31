cmake_minimum_required(VERSION 3.31)
project(Spin C)

set(CMAKE_C_STANDARD 11)

add_executable(Spin main.c)
target_link_libraries(Spin m)