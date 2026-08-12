# SPDX-License-Identifier: LGPL-2.1-or-later

# cli tools
add_subdirectory(cli)

# program name
set(programName h900edit)

# dependencies (external)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Svg LinguistTools
                 OPTIONAL_COMPONENTS
)
find_package(Boost CONFIG REQUIRED)

set(resourcePath "../res/")

# setup
qt_standard_project_setup(REQUIRES 6.8
    I18N_SOURCE_LANGUAGE en
    I18N_TRANSLATED_LANGUAGES de
)

# sources
if(WIN32)
set(sources 
  ${resourcePath}/icon.rc
)
endif()
set(sources ${sources} 
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
  document/data/cmd/moveActivity.cpp
  document/data/cmd/setIr.cpp
  document/data/cmd/setIrProto.cpp
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
  models/activityListModel.cpp
  models/baseListModel.cpp
  models/deviceListModel.cpp
  models/buttonListModel.cpp
  models/protocolIrListModel.cpp
  models/rawIrListModel.cpp
  ui/delegates/combobox.cpp
  ui/delegates/rawIr.cpp
  ui/delegates/protocolIr.cpp
  ui/delegates/uidCombobox.cpp
  ui/editors/codeEditor.cpp
  ui/editors/rawIrEditor.cpp
  ui/activityButtonTreeView.cpp
  ui/activityEditor.cpp
  ui/activityTreeView.cpp
  ui/activityEditor.cpp
  ui/baseEditor.cpp
  ui/baseTreeView.cpp
  ui/commandEditorView.cpp
  ui/concordConnection.cpp
  ui/deviceButtonTreeView.cpp
  ui/deviceEditor.cpp
  ui/deviceTreeView.cpp
  ui/logViewer.cpp
  ui/settings.cpp
  wrappers/concordWrapper.cpp
)

# executable
qt_add_executable(${programName} ${sources})

add_dependencies(${programName} libconcord_ext concordance_exe)
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
file(GLOB_RECURSE buttonIcons RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${resourcePath}buttons/*.jpg")
file(GLOB_RECURSE h900platform RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${resourcePath}h900platform/*")
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
    ${buttonIcons}
    ${h900platform}
)

qt_add_resources(${programName} "application"
    PREFIX
        "/"
    FILES
        ${${programName}_resource_files}
)

# cpack / install
if(WIN32)
  # mxe cross build
  set(mxeSourcePath /opt/mxe/usr/x86_64-w64-mingw32.shared)
  
  #editor
  install(TARGETS ${programName}
      RUNTIME DESTINATION .
  )
  #tools
  install(TARGETS h900irProtoTool h900learnTool h900ssIrTool
      RUNTIME DESTINATION .
  )
  install(FILES 
      "${CMAKE_BINARY_DIR}/concordance_exe/build/concordance.exe"
      DESTINATION .
  )  
  #libs
  install(FILES
      "${CMAKE_BINARY_DIR}/x64/bin/libqtadvanceddocking-qt6.dll"
      "${CMAKE_BINARY_DIR}/libconcord/build/.libs/libconcord-6.dll"
      "${CMAKE_BINARY_DIR}/src_external/pugixml/libpugixml.dll"
      DESTINATION .      
  )
  #syslibs
  install(FILES
      $<TARGET_FILE:Qt6::Core>
      $<TARGET_FILE:Qt6::Gui>
      $<TARGET_FILE:Qt6::Widgets>
      $<TARGET_FILE:Qt6::Svg>
      ${mxeSourcePath}/bin/icudt74.dll
      ${mxeSourcePath}/bin/icuin74.dll
      ${mxeSourcePath}/bin/icuuc74.dll
      ${mxeSourcePath}/bin/libbrotlicommon.dll
      ${mxeSourcePath}/bin/libbrotlidec.dll
      ${mxeSourcePath}/bin/libbz2.dll
      ${mxeSourcePath}/bin/libcurl-4.dll
      ${mxeSourcePath}/bin/libfreetype-6.dll
      ${mxeSourcePath}/bin/libgcc_s_seh-1.dll
      ${mxeSourcePath}/bin/libglib-2.0-0.dll
      ${mxeSourcePath}/bin/libharfbuzz-0.dll
      ${mxeSourcePath}/bin/libhidapi.dll
      ${mxeSourcePath}/bin/libiconv-2.dll
      ${mxeSourcePath}/bin/libidn2-0.dll
      ${mxeSourcePath}/bin/libintl-8.dll
      ${mxeSourcePath}/bin/libnghttp2-14.dll
      ${mxeSourcePath}/bin/libpcre2-16-0.dll
      ${mxeSourcePath}/bin/libpcre2-8-0.dll
      ${mxeSourcePath}/bin/libpng16-16.dll
      ${mxeSourcePath}/bin/libpsl-5.dll
      ${mxeSourcePath}/bin/libssh2-1.dll
      ${mxeSourcePath}/bin/libstdc++-6.dll
      ${mxeSourcePath}/bin/libunistring-5.dll
      ${mxeSourcePath}/bin/libwinpthread-1.dll
      ${mxeSourcePath}/bin/libzip.dll
      ${mxeSourcePath}/bin/libzstd.dll
      ${mxeSourcePath}/bin/zlib1.dll
      DESTINATION .   
  )
  install(FILES
      $<TARGET_FILE:Qt6::QWindowsIntegrationPlugin>
      DESTINATION platforms
  )
  install(FILES
      ${mxeSourcePath}/qt6/plugins/imageformats/qsvg.dll
      DESTINATION plugins/imageformats
  )
else()
  #setup install target. use with e.g make DESTDIR=./install install
  install(TARGETS ${programName}
      RUNTIME DESTINATION bin
  )
  install(TARGETS qtadvanceddocking-qt6
      RUNTIME DESTINATION bin
  )
  install(FILES "${CMAKE_SOURCE_DIR}/res/h900edit.desktop"
      DESTINATION share/applications
  )
  install(FILES "${resourcePath}/harmony-900-color-screen.png"
      DESTINATION share/icons/hicolor/64x64/apps
      RENAME h900edit.png
  )
  install(DIRECTORY "${LIBCONCORD_INSTALL}/lib/"
      DESTINATION lib
      FILES_MATCHING PATTERN "libconcord.so*"
  )
endif()

# cpack config
set(CPACK_PACKAGE_NAME "h900tools")
set(CPACK_PACKAGE_VENDOR "Martin Wagner")
set(CPACK_PACKAGE_CONTACT "https://github.com/martin-wagner/harmony900tools/issues")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "H900 remote tools")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
include("${VERSION_CMAKE_OUTPUT}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_GENERATOR "TGZ") #always tarball

if(WIN32)
  list(APPEND CPACK_GENERATOR "NSIS;ZIP")

  set(CPACK_NSIS_PACKAGE_NAME "${programName}")
  set(CPACK_NSIS_DISPLAY_NAME "${programName}")
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
  set(CPACK_NSIS_MODIFY_PATH OFF)
  set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
  set(CPACK_NSIS_CREATE_ICONS_EXTRA
      "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\${programName}.lnk' '$INSTDIR\\\\${programName}.exe'"
  )
  set(CPACK_NSIS_DELETE_ICONS_EXTRA
      "Delete '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\${programName}.lnk'"
  )
  set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
else()
  list(APPEND CPACK_GENERATOR "DEB")

  set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR}")
  set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
  set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/martin-wagner/harmony900tools")
  set(CPACK_DEBIAN_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})
endif()

#always last!
include(CPack)
