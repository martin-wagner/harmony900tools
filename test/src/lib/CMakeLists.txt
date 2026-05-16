# SPDX-License-Identifier: LGPL-2.1-or-later

# ---------------------------------------------------------------------------
# Source location
# ---------------------------------------------------------------------------
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src/lib)

# ---------------------------------------------------------------------------
# Qt — add this if not already in a parent CMakeLists.txt
# ---------------------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
qt_standard_project_setup()

# ---------------------------------------------------------------------------
# test_libs
# ---------------------------------------------------------------------------
qt_add_executable(test_libs
    ${SRC_DIR}/undo.cpp
    test_undo.cpp
)

target_include_directories(test_libs
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${SRC_DIR}
)

target_link_libraries(test_libs
    PRIVATE
        GTest::gtest_main
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

# Inject path to the real irProto.bin so the fixture can find it
target_compile_definitions(test_libs PRIVATE
    REAL_IRPROTO_BIN_PATH="${CMAKE_CURRENT_SOURCE_DIR}/IrProto.bin"
)

# ---------------------------------------------------------------------------
# add to test suite
# ---------------------------------------------------------------------------
gtest_discover_tests(test_libs)
