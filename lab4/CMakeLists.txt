cmake_minimum_required(VERSION 3.18.4)
project(server)
set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)
add_executable(server
        src/server/main.cpp
        open_tcp_connection/open_tcp_connection.hpp
        open_tcp_connection/open_tcp_connection.cpp
        socket_read/socket_read.hpp
        socket_read/socket_read.cpp)
target_compile_options(server PUBLIC "-pthread")
target_link_libraries(server "${CMAKE_THREAD_LIBS_INIT}")
add_executable(client
        src/client/main.cpp
        open_tcp_connection/open_tcp_connection.hpp
        open_tcp_connection/open_tcp_connection.cpp
        socket_read/socket_read.hpp
        socket_read/socket_read.cpp)
