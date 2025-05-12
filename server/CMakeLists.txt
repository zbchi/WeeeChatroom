cmake_minimum_required(VERSION 3.10)

add_executable(chat_server 
    main.cc
    Service.cc
    login.cc
    redis.cc
)


find_package(CURL REQUIRED)

find_path(HIREDIS_INCLUDE_DIR hiredis/hiredis.h)
find_library(HIREDIS_LIB hiredis)


target_link_libraries(chat_server PRIVATE 
    netlib 
    CURL::libcurl  # 使用现代 CMake 的 CURL 目标
    ${HIREDIS_LIB}
)

target_include_directories(chat_server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${HIREDIS_INCLUDE_DIR}
)