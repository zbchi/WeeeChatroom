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

get_filename_component(PROJECT_ROOT_DIR "${CMAKE_SOURCE_DIR}/.." ABSOLUTE)
target_compile_definitions(chat_client PRIVATE PROJECT_ROOT_DIR="${PROJECT_ROOT_DIR}")