# SPDX-License-Identifier: LGPL-2.1-or-later

set(sources
    irProto.cpp
    ${CMAKE_SOURCE_DIR}/src/bin/irProto/file.cpp
    ${CMAKE_SOURCE_DIR}/src/bin/irProto/code.cpp
    ${CMAKE_SOURCE_DIR}/src/bin/timing.cpp
)

add_executable(h900irProtoTool ${sources})
target_include_directories(h900irProtoTool PRIVATE ${CMAKE_SOURCE_DIR}/src)
if(WIN32)
    target_compile_options(h900irProtoTool PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
