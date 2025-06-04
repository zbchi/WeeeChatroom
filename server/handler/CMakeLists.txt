add_library(handler_lib STATIC
Loginer.cc
Register.cc
Friender.cc
Chatter.cc
Grouper.cc
)

include_directories(/user/include/mysql)
find_package(CURL REQUIRED)
find_path(HIREDIS_INCLUDE_DIR hiredis/hiredis.h)
find_library(HIREDIS_LIB hiredis)

target_include_directories(handler_lib PRIVATE 
${CMAKE_CURRENT_SOURCE_DIR}/include
${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_link_libraries(handler_lib PRIVATE 
netlib
CURL::libcurl
${HIREDIS_LIB}
mysqlclient
)