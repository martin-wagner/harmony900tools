set(sources
    ssIr.cpp
    file.cpp
    ${CMAKE_SOURCE_DIR}/src/cli/lib/data.cpp
)

add_executable(harmony900ssIrTool ${sources})
target_include_directories(harmony900ssIrTool PRIVATE ${CMAKE_SOURCE_DIR}/src)

