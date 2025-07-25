project(chat_client)

add_subdirectory(service)

add_executable(chat_client 
    main.cc
    Neter.cc
    Controller.cc
    Client.cc
    ui.cc
)


target_link_libraries(chat_client PRIVATE 
    netlib
    service_lib
    readline
     #profiler
   # pthread
)

target_include_directories(chat_client PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)