include(FetchContent)
FetchContent_Declare(
    pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    GIT_TAG        ee86beb #v1.15
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/src_external/pugixml
    BINARY_DIR     ${CMAKE_BINARY_DIR}/src_external/pugixml
)

# Disable unnecessary targets
set(PUGIXML_BUILD_TESTS OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(pugixml)
