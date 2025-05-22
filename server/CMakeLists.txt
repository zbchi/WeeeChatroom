cmake_minimum_required(VERSION 3.10)

project(chat_server)

add_subdirectory(handler)

add_executable(chat_server 
    main.cc
    Redis.cc
    base.cc
    MySQLConn.cc
    ThreadPool.cc
    Service.cc
    )


include_directories(/user/include/mysql)
find_package(CURL REQUIRED)
find_path(HIREDIS_INCLUDE_DIR hiredis/hiredis.h)
find_library(HIREDIS_LIB hiredis)


target_link_libraries(chat_server PRIVATE 
    netlib 
    CURL::libcurl
    ${HIREDIS_LIB}
    mysqlclient
    handler_lib
)

target_include_directories(chat_server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/handler/include
    ${HIREDIS_INCLUDE_DIR}
)