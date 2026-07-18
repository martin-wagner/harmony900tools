# SPDX-License-Identifier: LGPL-2.1-or-later

# ---------------------------------------------------------------------------
# Source location
# ---------------------------------------------------------------------------
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src/document/data)

# ---------------------------------------------------------------------------
# Qt — add this if not already in a parent CMakeLists.txt
# ---------------------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Core)  # QString / QList

# ---------------------------------------------------------------------------
# test_strEnum
# ---------------------------------------------------------------------------
add_executable(test_strEnum
    test_enum.cpp
    ${SRC_DIR}/enum.cpp
)

target_include_directories(test_strEnum
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${SRC_DIR}
)

target_link_libraries(test_strEnum
    PRIVATE
        GTest::gtest_main
        magic_enum::magic_enum
        Qt6::Core
)

# Inject path to the real irProto.bin so the fixture can find it
target_compile_definitions(test_strEnum PRIVATE
    REAL_IRPROTO_BIN_PATH="${CMAKE_CURRENT_SOURCE_DIR}/IrProto.bin"
)


# ---------------------------------------------------------------------------
# add to test suite
# ---------------------------------------------------------------------------
gtest_discover_tests(test_strEnum)


add_subdirectory(items)