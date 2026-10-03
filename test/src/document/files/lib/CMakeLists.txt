# SPDX-License-Identifier: LGPL-2.1-or-later

# ---------------------------------------------------------------------------
# Source location
# ---------------------------------------------------------------------------
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src/document/files/lib)

# ---------------------------------------------------------------------------
# Qt — add this if not already in a parent CMakeLists.txt
# ---------------------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Core)  # QString / QList

# ---------------------------------------------------------------------------
# test
# ---------------------------------------------------------------------------
add_executable(test_keyCode
    test_keyCode.cpp
    ${SRC_DIR}/keyCode.cpp
)

target_include_directories(test_keyCode
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${SRC_DIR}
)

target_link_libraries(test_keyCode
    PRIVATE
        GTest::gtest_main
        Qt6::Core
)

# Inject path to the real irProto.bin so the fixture can find it
target_compile_definitions(test_keyCode PRIVATE
    KEY_CODE_TEST_PATH="${CMAKE_CURRENT_SOURCE_DIR}/keyCodes.txt"
)


# ---------------------------------------------------------------------------
# add to test suite
# ---------------------------------------------------------------------------
gtest_discover_tests(test_keyCode)
