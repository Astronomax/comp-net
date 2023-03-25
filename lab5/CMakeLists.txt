cmake_minimum_required(VERSION 3.18.4)
project(task_1)
set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)
add_executable(task_1
        "src/task_1/main.cpp"
        open_tcp_connection/open_tcp_connection.hpp
        open_tcp_connection/open_tcp_connection.cpp)
include_directories(${CURL_INCLUDE_DIR})
target_link_libraries(task_1 ${CURL_LIBRARIES})
