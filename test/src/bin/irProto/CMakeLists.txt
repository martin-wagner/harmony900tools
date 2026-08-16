# SPDX-License-Identifier: LGPL-2.1-or-later

# ---------------------------------------------------------------------------
# Qt — add this if not already in a parent CMakeLists.txt
# ---------------------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Core)  # QString / QList

# ---------------------------------------------------------------------------
# Source location
# ---------------------------------------------------------------------------
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src/bin/irProto)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/src/bin)

# ---------------------------------------------------------------------------
# test_irProto
# ---------------------------------------------------------------------------
add_executable(test_irProto
    test_file.cpp
    test_code.cpp
    test_decode.cpp
    ${LIB_DIR}/timing.cpp
    ${SRC_DIR}/file.cpp
    ${SRC_DIR}/code.cpp
    ${SRC_DIR}/decode.cpp
    ${CMAKE_SOURCE_DIR}/src/document/data/enum.cpp
)

target_include_directories(test_irProto
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${SRC_DIR}
)

target_link_libraries(test_irProto
    PRIVATE
        GTest::gtest_main
        magic_enum::magic_enum
        IRremoteESP8266
        Qt6::Core
)

# Inject paths for test files
target_compile_definitions(test_irProto PRIVATE
    ENUMSONLY
    REAL_IRPROTO_BIN_PATH="${CMAKE_CURRENT_SOURCE_DIR}/IrProto.bin"
    TEST_CONFIG_PATH="${CMAKE_SOURCE_DIR}/res/ir_protocols.json"
)

# ---------------------------------------------------------------------------
# add to test suite
# ---------------------------------------------------------------------------
gtest_discover_tests(test_irProto)
