add_library(service_lib STATIC
ChatService.cc
FriendService.cc
GroupService.cc
UserService.cc
)


target_include_directories(service_lib PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}/include
${CMAKE_CURRENT_SOURCE_DIR}/../include
)


target_link_libraries(service_lib PRIVATE 
netlib
)