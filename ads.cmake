include(FetchContent)
FetchContent_Declare(
    QtADS
    GIT_REPOSITORY https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System.git
    GIT_TAG        985ff74ccf0bb9fc2bea55f2a3be91568488f8dd
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/src_external/Qt-Advanced-Docking-System
    BINARY_DIR     ${CMAKE_BINARY_DIR}/src_external/Qt-Advanced-Docking-System
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(QtADS)