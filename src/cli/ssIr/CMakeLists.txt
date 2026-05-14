# SPDX-License-Identifier: LGPL-2.1-or-later

set(sources
    ssIr.cpp
    ${CMAKE_SOURCE_DIR}/src/bin/ssIr/file.cpp
    ${CMAKE_SOURCE_DIR}/src/bin/data.cpp
)

add_executable(harmony900ssIrTool ${sources})
target_include_directories(harmony900ssIrTool PRIVATE ${CMAKE_SOURCE_DIR}/src)
if(WIN32)
    target_compile_options(harmony900ssIrTool PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
