include(FetchContent)
FetchContent_Declare(
    pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    GIT_TAG        c8033ce9d039e7f9d134877c363397b3cfe20816 #v1.16
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/src_external/pugixml
    BINARY_DIR     ${CMAKE_BINARY_DIR}/src_external/pugixml
)

# Disable unnecessary targets
set(PUGIXML_BUILD_TESTS OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(pugixml)
