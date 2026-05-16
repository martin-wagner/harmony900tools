include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        55f93686c01528224f448c19128836e7df245f72 #v3.12
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/src_external/nlohmann_json
    BINARY_DIR     ${CMAKE_BINARY_DIR}/src_external/nlohmann_json
)

# Disable unnecessary targets
set(JSON_BuildTests      OFF CACHE INTERNAL "")
set(JSON_Install         OFF CACHE INTERNAL "")
set(JSON_MultipleHeaders OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(nlohmann_json)
