set(sources
    irProto.cpp
#    file.cpp
    ${CMAKE_SOURCE_DIR}/src/cli/lib/data.cpp
)

add_executable(harmony900irProtoTool ${sources})
target_include_directories(harmony900irProtoTool PRIVATE ${CMAKE_SOURCE_DIR}/src)

