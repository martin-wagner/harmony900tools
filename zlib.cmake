include(FetchContent)
FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG        da607da739fa6047df13e66a2af6b8bec7c2a498 #v1.3.2
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/src_external/zlib
    BINARY_DIR     ${CMAKE_BINARY_DIR}/src_external/zlib
)

set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(zlib)

# contrib/minizip has no CMakeLists — build sources directly
add_library(minizip STATIC
    ${zlib_SOURCE_DIR}/contrib/minizip/ioapi.c
    ${zlib_SOURCE_DIR}/contrib/minizip/zip.c
    ${zlib_SOURCE_DIR}/contrib/minizip/unzip.c
    $<$<PLATFORM_ID:Windows>:${zlib_SOURCE_DIR}/contrib/minizip/iowin32.c>
)

target_include_directories(minizip PUBLIC
    ${zlib_SOURCE_DIR}/contrib/
    ${zlib_SOURCE_DIR}
    ${zlib_BINARY_DIR}    # zconf.h is generated here
)

target_link_libraries(minizip PUBLIC zlibstatic)