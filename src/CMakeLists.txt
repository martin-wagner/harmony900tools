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
  lib/undo.cpp
  lib/zip.cpp
  main.cpp
  mainwindow.cpp
  bin/timing.cpp
  bin/irProto/code.cpp
  bin/irProto/file.cpp
  bin/ssIr/file.cpp
  comm/concord.cpp
  document/config.cpp
  document/data/catalogue.cpp
  document/data/data.cpp
  document/data/enum.cpp
  document/data/cmd/base.cpp
  document/data/cmd/addActionSequence.cpp
  document/data/cmd/addActivity.cpp
  document/data/cmd/addActivityAction.cpp
  document/data/cmd/addButton.cpp
  document/data/cmd/addChannel.cpp
  document/data/cmd/addDevice.cpp
  document/data/cmd/addNumpad.cpp
  document/data/cmd/addPowerOnDevice.cpp
  document/data/cmd/addState.cpp
  document/data/cmd/addStatemachine.cpp
  document/data/cmd/setIr.cpp
  document/data/cmd/setIrProto.cpp
  document/data/cmd/setIrStream.cpp
  document/data/cmd/setRole.cpp
  document/data/cmd/setId.cpp
  document/data/cmd/removeActionSequence.cpp
  document/data/cmd/removeActivity.cpp
  document/data/cmd/removeActivityAction.cpp
  document/data/cmd/removeButton.cpp
  document/data/cmd/removeChannel.cpp  
  document/data/cmd/removeDevice.cpp
  document/data/cmd/removeIr.cpp
  document/data/cmd/removeIrProto.cpp
  document/data/cmd/removeIrStream.cpp
  document/data/cmd/removeNumpad.cpp
  document/data/cmd/removePowerOnDevice.cpp
  document/data/cmd/removeRole.cpp
  document/data/cmd/removeState.cpp
  document/data/cmd/removeStatemachine.cpp
  document/data/cmd/setControllerMetadata.cpp
  document/data/cmd/setUnknownProperty.cpp
  document/data/cmd/setUserData.cpp
  document/data/items/button.cpp
  document/data/items/activityJson.cpp
  document/data/items/codeJson.cpp
  document/data/items/commonJson.cpp
  document/data/items/deviceJson.cpp
  document/data/items/irProtoJson.cpp
  document/data/items/ssIrJson.cpp
  document/files/create.cpp
  document/files/h900.cpp
  document/files/storage.cpp
  models/deviceListModel.cpp
  ui/logViewer.cpp
  ui/settings.cpp
  ui/concordConnection.cpp
  ui/deviceEditor.cpp
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
    minizip
    qtadvanceddocking-qt6
    Libconcord::libconcord
    nlohmann_json::nlohmann_json
    pugixml::pugixml
    magic_enum::magic_enum
)

if(WIN32)
  target_compile_definitions(${programName} PRIVATE SECURITY_WIN32=1)
  target_link_libraries(${programName} PRIVATE secur32)
endif()

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
    ${iconPath}/actions/document-import.png
    ${iconPath}/actions/document-save.png
    ${iconPath}/actions/document-save-as.png
    ${iconPath}/actions/application-exit.png
    ${iconPath}/actions/edit-undo.png
    ${iconPath}/actions/edit-redo.png
    ${iconPath}/actions/network-connect.png
    ${iconPath}/actions/network-disconnect.png
    ${iconPath}/status/dialog-information.png
    ${iconPath}/misc/org.kde.plasma.analogclock.png
    ${iconPath}/actions/kt-set-max-upload-speed.png
    ${iconPath}/actions/backup.png
    ${iconPath}/actions/edit-download.png
    ${iconPath}/actions/media-record.png
    ${iconPath}/actions/keyframe-record.png
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
