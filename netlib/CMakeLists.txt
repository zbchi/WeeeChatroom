file(GLOB BASE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/base/*.cc)
file(GLOB REACTOR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/reactor/*.cc)

add_library(netlib STATIC ${BASE_SRC} ${REACTOR_SRC})

target_include_directories(netlib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/base
    ${CMAKE_CURRENT_SOURCE_DIR}/reactor
    )