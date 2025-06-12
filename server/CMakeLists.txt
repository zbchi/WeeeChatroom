project(chat_server)

add_subdirectory(handler)
add_subdirectory(base)

add_executable(chat_server 
    main.cc
    Service.cc
    )

target_link_libraries(chat_server PRIVATE 
    handler_lib
    base_lib
)
