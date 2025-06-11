add_library(handler_lib STATIC
Loginer.cc
Register.cc
Friender.cc
Chatter.cc
Grouper.cc
)


target_include_directories(handler_lib PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}/include
${CMAKE_CURRENT_SOURCE_DIR}/../include
)


target_link_libraries(handler_lib PRIVATE 
base_lib
)