cmake_minimum_required(VERSION 3.15)
project(ping)

set(CMAKE_CXX_STANDARD 17)

add_executable(ping main.cpp)

if(WIN32)
    target_link_libraries(ping wsock32 ws2_32)
endif()