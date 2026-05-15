# SPDX-License-Identifier: LGPL-2.1-or-later

# cli tools
add_subdirectory(cli)

# program name
set(programName h900tool)

#if(NOT DEFINED INSTALL_EXAMPLESDIR)
#    set(INSTALL_EXAMPLESDIR "examples")
#endif()

#set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/widgets/mainwindows/application")

# dependencies (external)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg LinguistTools
                 OPTIONAL_COMPONENTS
)
find_package(Boost CONFIG REQUIRED)

# setup
qt_standard_project_setup(REQUIRES 6.8
    I18N_SOURCE_LANGUAGE en
    I18N_TRANSLATED_LANGUAGES de
)

# sources
set(sources 
  lib/users.h  #moc for headers
  main.cpp
  mainwindow.cpp
  bin/timing.cpp
  comm/concord.cpp
  document/domain/data.cpp
  document/domain/enum.cpp
  document/domain/items/action.cpp
  document/domain/items/activity.cpp
  ui/logViewer.cpp
  ui/settings.cpp
  ui/concordTest.cpp
  wrappers/concordWrapper.cpp
)

# executable
qt_add_executable(${programName} ${sources})

add_dependencies(${programName} libconcord_ext)
add_dependencies(${programName} version_info)

if(ltoEnabled)
  set_property(TARGET ${programName} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

set_target_properties(${programName} PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_include_directories(${programName} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${programName} PRIVATE "${CMAKE_BINARY_DIR}/generated")
target_compile_definitions(${programName} PRIVATE PROGRAM_NAME="${programName}")
target_compile_definitions(${programName} PRIVATE QT_DEBUG_NO_OUTPUT)

target_link_libraries(${programName} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
    qtadvanceddocking-qt6
    Libconcord::libconcord
    nlohmann_json::nlohmann_json
    pugixml::pugixml
    magic_enum::magic_enum
)

# Resources:
set(resourcePath "../res/")
set(iconPath "${resourcePath}icons/BreezeConverted/64x64/")
set(${programName}_resource_files
    ${resourcePath}/splash.png
    ${resourcePath}/harmony-900-color-screen.png
    ${iconPath}/actions/edit-copy.png
    ${iconPath}/actions/edit-paste.png
    ${iconPath}/actions/edit-cut.png
    ${iconPath}/actions/edit-clear.png
    ${iconPath}/actions/gnumeric-object-scrollbar.png
    ${iconPath}/actions/document-new.png
    ${iconPath}/actions/document-open.png
    ${iconPath}/actions/document-save.png
    ${iconPath}/actions/document-save-as.png
    ${iconPath}/actions/application-exit.png
)

qt_add_resources(${programName} "application"
    PREFIX
        "/"
    FILES
        ${${programName}_resource_files}
)

#install(TARGETS application
#    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
#    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
#    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
#)

#always last!
#include(CPack)
