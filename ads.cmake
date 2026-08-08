include(FetchContent)
FetchContent_Declare(
    QtADS
    GIT_REPOSITORY https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System.git
    GIT_TAG        433b0c90b44d8f17d059204176ff6e0d6d3783e3 #v5.0
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/src_external/Qt-Advanced-Docking-System
    BINARY_DIR     ${CMAKE_BINARY_DIR}/src_external/Qt-Advanced-Docking-System
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(QT_VERSION_MAJOR 6)
FetchContent_MakeAvailable(QtADS)
