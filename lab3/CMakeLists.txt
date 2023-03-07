cmake_minimum_required(VERSION 3.23)
project(server)
set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)
add_executable(server server/main.cpp)
target_compile_options(server PUBLIC "-pthread")
target_link_libraries(server "${CMAKE_THREAD_LIBS_INIT}")
add_executable(client client/main.cpp)
