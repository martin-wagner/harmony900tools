set(sources
    learn.cpp
    trx_base.cpp
    trx_single.cpp
    trx_stream.cpp
    data.cpp
)

add_executable(harmony900learnTool ${sources})
target_include_directories(harmony900learnTool PRIVATE ${CMAKE_SOURCE_DIR}/src)

