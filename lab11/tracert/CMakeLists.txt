cmake_minimum_required(VERSION 3.15)
project(tracert)

set(CMAKE_CXX_STANDARD 17)

add_executable(tracert main.cpp)

if(WIN32)
    target_link_libraries(tracert wsock32 ws2_32)
endif()