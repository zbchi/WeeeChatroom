add_library(base_lib STATIC
    base.cc
    MySQLConn.cc
    Redis.cc
    ThreadPool.cc
)

include_directories(/usr/include/mysql)

# CURL
find_package(CURL REQUIRED)

# 手动添加 hiredis 的头文件和库路径
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

# redis++
include_directories(/usr/local/include)  # redis++ 头文件

target_include_directories(base_lib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(base_lib PUBLIC
    netlib
    CURL::libcurl
    /usr/local/lib/libredis++.a
    /usr/local/lib/libhiredis.a
    mysqlclient
    uuid
)
