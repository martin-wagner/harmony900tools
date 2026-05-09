set(sources
    irProto.cpp
    ${CMAKE_SOURCE_DIR}/src/bin/irProto/file.cpp
    ${CMAKE_SOURCE_DIR}/src/bin/irProto/code.cpp
    ${CMAKE_SOURCE_DIR}/src/bin/data.cpp
)

add_executable(harmony900irProtoTool ${sources})
target_include_directories(harmony900irProtoTool PRIVATE ${CMAKE_SOURCE_DIR}/src)
if(WIN32)
    target_compile_options(harmony900irProtoTool PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
