add_library(handler_lib STATIC
Login.cc
Register.cc
Friend.cc
Chat.cc
Group.cc
File.cc
)


target_include_directories(handler_lib PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}/include
${CMAKE_CURRENT_SOURCE_DIR}/..
)


target_link_libraries(handler_lib PRIVATE 
base_lib
)