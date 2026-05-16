include(FetchContent)
FetchContent_Declare(
    magic_enum
    GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
    GIT_TAG        1384769c66bd16ec9bb1353f45fe8ec8ccc12dbd #v0.9.8
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/src_external/magic_enum
    BINARY_DIR     ${CMAKE_BINARY_DIR}/src_external/magic_enum
)

FetchContent_MakeAvailable(magic_enum)

add_compile_definitions(MAGIC_ENUM_ENABLE_HASH)
