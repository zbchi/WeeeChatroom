cmake_minimum_required(VERSION 3.10)
project(ChatRoom)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

option(O "Only build client" OFF)

add_subdirectory(netlib)

if (NOT O)
    add_subdirectory(server)
endif()

add_subdirectory(client)                                   