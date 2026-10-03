# SPDX-License-Identifier: LGPL-2.1-or-later

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(sources
    database.cpp
    ${CMAKE_SOURCE_DIR}/src/document/data/enum.cpp
    ${CMAKE_SOURCE_DIR}/src/bin/irProto/file.cpp
    ${CMAKE_SOURCE_DIR}/src/bin/irProto/code.cpp
    ${CMAKE_SOURCE_DIR}/src/bin/timing.cpp
    ${CMAKE_SOURCE_DIR}/src/document/files/database/importer.cpp
    ${CMAKE_SOURCE_DIR}/src/document/files/database/keyCode.cpp
)

add_executable(h900databaseTool ${sources})
set_target_properties(h900databaseTool PROPERTIES AUTOMOC ON)
target_include_directories(h900databaseTool PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(h900databaseTool PRIVATE 
    Qt6::Core
    Qt6::Widgets
    magic_enum::magic_enum
    nlohmann_json::nlohmann_json
)
if(WIN32)
    target_compile_options(h900databaseTool PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()