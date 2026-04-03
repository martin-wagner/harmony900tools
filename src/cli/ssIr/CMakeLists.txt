set(sources
    ssIr.cpp
)

add_executable(harmony900ssIrTool ${sources})
target_include_directories(harmony900ssIrTool PRIVATE ${CMAKE_SOURCE_DIR}/src)

