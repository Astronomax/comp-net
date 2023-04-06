cmake_minimum_required(VERSION 3.23)
project(ftp_client)

set(CMAKE_CXX_STANDARD 17)

add_executable(ftp_client main.cpp
        open_tcp_connection/open_tcp_connection.hpp
        open_tcp_connection/open_tcp_connection.cpp
        uri/uri.hpp
        uri/uri.cpp)
