# SPDX-License-Identifier: LGPL-2.1-or-later

# ---------------------------------------------------------------------------
# Fetch GoogleTest
# ---------------------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        52eb810 #v1.17.0
    SOURCE_DIR     ${CMAKE_SOURCE_DIR}/src_external/googletest
    BINARY_DIR     ${CMAKE_BINARY_DIR}/src_external/googletest
)

# Don't install gtest alongside the main project
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)
include(GoogleTest)

add_subdirectory(src)
