add_executable(chat_server 
main.cc
Service.cc
login.cc
)
target_link_libraries(chat_server PRIVATE netlib)
target_include_directories(chat_server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)