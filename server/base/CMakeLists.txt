add_library(base_lib STATIC
base.cc
MySQLConn.cc
Redis.cc
ThreadPool.cc
)

include_directories(/user/include/mysql)
find_package(CURL REQUIRED)
find_path(HIREDIS_INCLUDE_DIR hiredis/hiredis.h)
find_library(HIREDIS_LIB hiredis)

target_include_directories(base_lib PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}/include
${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_link_libraries(base_lib PUBLIC
netlib
CURL::libcurl
${HIREDIS_LIB}
mysqlclient
)