include(FetchContent)
FetchContent_Declare(
    QtADS
    GIT_REPOSITORY https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System.git
    GIT_TAG        023ce95934fecfd4cf5c672c9c404fabe0f54923 #v5.1.1
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/src_external/Qt-Advanced-Docking-System
    BINARY_DIR     ${CMAKE_BINARY_DIR}/src_external/Qt-Advanced-Docking-System
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(QT_VERSION_MAJOR 6)
FetchContent_MakeAvailable(QtADS)
