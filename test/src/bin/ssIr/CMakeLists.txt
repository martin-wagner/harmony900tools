# SPDX-License-Identifier: LGPL-2.1-or-later

# ---------------------------------------------------------------------------
# Source location
# ---------------------------------------------------------------------------
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src/bin/ssIr)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/src/bin)

# ---------------------------------------------------------------------------
# test_ssIr
# ---------------------------------------------------------------------------
add_executable(test_ssIr
    test_file.cpp
    ${LIB_DIR}/data.cpp
    ${SRC_DIR}/file.cpp
)

target_include_directories(test_ssIr
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${SRC_DIR}
)

target_link_libraries(test_ssIr
    PRIVATE
        GTest::gtest_main
)

# Inject path to the real SsIr.bin so the fixture can find it
target_compile_definitions(test_ssIr PRIVATE
    REAL_SSIR_BIN_PATH="${CMAKE_CURRENT_SOURCE_DIR}/SsIr.bin"
)


# ---------------------------------------------------------------------------
# add to test suite
# ---------------------------------------------------------------------------
gtest_discover_tests(test_ssIr)
