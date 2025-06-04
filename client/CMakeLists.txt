add_executable(chat_client 
main.cc
Neter.cc
UserService.cc
ChatService.cc
FriendService.cc
GroupService.cc

Controller.cc
Client.cc
)

target_link_libraries(chat_client PRIVATE netlib)

target_include_directories(chat_client PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)