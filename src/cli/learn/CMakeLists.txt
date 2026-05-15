# SPDX-License-Identifier: LGPL-2.1-or-later

set(sources
    learn.cpp
    ${CMAKE_SOURCE_DIR}/src/bin/learn/trx_base.cpp
    ${CMAKE_SOURCE_DIR}/src/bin/learn/trx_single.cpp
    ${CMAKE_SOURCE_DIR}/src/bin/learn/trx_stream.cpp
    ${CMAKE_SOURCE_DIR}/src/bin/timing.cpp
)

add_executable(harmony900learnTool ${sources})
target_include_directories(harmony900learnTool PRIVATE ${CMAKE_SOURCE_DIR}/src)
if(WIN32)
    target_link_libraries(harmony900learnTool PRIVATE ws2_32)
    target_compile_options(harmony900learnTool PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
