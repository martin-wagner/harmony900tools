# decodeir.cmake
#
# Provides two targets:
#   decodeir      - static library with the decode logic, no main(), for linking
#                   into your own executable.
#   decodeir_cli  - the original standalone CLI tool, built exactly as the
#                   upstream Makefile intended (raw/Pronto hex on the command line).

include(FetchContent)

FetchContent_Declare(
    decodeir
    GIT_REPOSITORY https://github.com/probonopd/decodeir.git
    GIT_TAG 37fd7067b0f686fdcc066101489b4f6c4688fe07
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/src_external/decodeIr
    BINARY_DIR     ${CMAKE_BINARY_DIR}/src_external/decodeIr
)

# decodeir has no CMakeLists.txt of its own (only a plain Makefile), so
# FetchContent_MakeAvailable() would fail trying add_subdirectory() on it.
# We fetch the source only and build our own targets from it.
FetchContent_GetProperties(decodeir)
if (NOT decodeir_POPULATED)
    FetchContent_Populate(decodeir)
endif()

# --- library target (for embedding in executable) ---
#
# DecodeIR.cpp bundles a CLI main() at the end of the file with no #ifdef guard,
# meant for their standalone Makefile executable. For the library we only want
# the decode logic, so strip everything from "int main(int argc" onward into a
# separate source file.
set(decodeirLibSource ${CMAKE_BINARY_DIR}/DecodeIR_lib.cpp)

file(READ ${decodeir_SOURCE_DIR}/DecodeIR.cpp decodeirSourceContent)
string(FIND "${decodeirSourceContent}" "int main(int argc" mainFunctionPos)
if (mainFunctionPos EQUAL -1)
    message(FATAL_ERROR "Could not find main() in DecodeIR.cpp to strip it out - upstream file may have changed, check manually")
endif()
string(SUBSTRING "${decodeirSourceContent}" 0 ${mainFunctionPos} decodeirLibContent)
file(WRITE ${decodeirLibSource} "${decodeirLibContent}")

add_library(decodeir STATIC
    ${decodeirLibSource}
)

target_include_directories(decodeir PUBLIC
    ${decodeir_SOURCE_DIR}
)

# --- original CLI executable, built as intended by upstream ---
add_executable(decodeir_cli
    ${decodeir_SOURCE_DIR}/DecodeIR.cpp
)

target_include_directories(decodeir_cli PRIVATE
    ${decodeir_SOURCE_DIR}
)

# old code, use old standard
foreach(decodeirTarget decodeir decodeir_cli)
    set_target_properties(${decodeirTarget} PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${decodeirTarget} PRIVATE -w)
    elseif (MSVC)
        target_compile_options(${decodeirTarget} PRIVATE /w)
    endif()
endforeach()