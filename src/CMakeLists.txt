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
find_package(Boost REQUIRED)

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
  ui/logViewer.cpp
  ui/settings.cpp
)

# executable
qt_add_executable(${programName} ${sources})

add_dependencies(${programName} libconcord_ext)
add_dependencies(${programName} qtadvanceddocking-qt6)

if(ltoEnabled)
  set_property(TARGET ${programName} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

set_target_properties(${programName} PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_include_directories(${programName} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(${programName} PRIVATE PROGRAM_NAME="${programName}")
target_compile_definitions(${programName} PRIVATE PROGRAM_VERSION="${PROJECT_VERSION}")
target_compile_definitions(${programName} PRIVATE QT_DEBUG_NO_OUTPUT)

target_link_libraries(${programName} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
    qtadvanceddocking-qt6
)

# Resources:
set(resourcePath "../res/")
set(${programName}_resource_files
    ${resourcePath}/copy.png
    ${resourcePath}/cut.png
    ${resourcePath}/new.png
    ${resourcePath}/open.png
    ${resourcePath}/paste.png
    ${resourcePath}/save.png
    ${resourcePath}/splash.png
    ${resourcePath}/icons8-scroll-lock-key-24.png
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
