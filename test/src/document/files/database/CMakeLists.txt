# SPDX-License-Identifier: LGPL-2.1-or-later

# ---------------------------------------------------------------------------
# Source location
# ---------------------------------------------------------------------------
set(DOCUMENT_DIR ${CMAKE_SOURCE_DIR}/src/document)
set(SRC_DIR ${DOCUMENT_DIR}/files/database)
set(BIN_DIR ${CMAKE_SOURCE_DIR}/src/bin)

# ---------------------------------------------------------------------------
# Qt — add this if not already in a parent CMakeLists.txt
# ---------------------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# ---------------------------------------------------------------------------
# test
# ---------------------------------------------------------------------------
add_executable(test_database
    test_keyCode.cpp
    test_jsonProtocols.cpp
    ${DOCUMENT_DIR}/data/enum.cpp
    ${SRC_DIR}/keyCode.cpp
    ${SRC_DIR}/jsonProtocols.cpp
    ${BIN_DIR}/timing.cpp
    ${BIN_DIR}/irProto/code.cpp
    ${BIN_DIR}/irProto/file.cpp
)

target_include_directories(test_database
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${SRC_DIR}
)

target_link_libraries(test_database
    PRIVATE
        GTest::gtest_main
        nlohmann_json::nlohmann_json
        magic_enum::magic_enum
        Qt6::Core
        Qt6::Widgets
)

# Inject path to the real irProto.bin so the fixture can find it
target_compile_definitions(test_database PRIVATE
    KEY_CODE_TEST_PATH="${CMAKE_CURRENT_SOURCE_DIR}/keyCodes.txt"
    ALL_PROTOCOLS_TEST_PATH="${CMAKE_CURRENT_SOURCE_DIR}/AllProtocols.json"
)


# ---------------------------------------------------------------------------
# add to test suite
# ---------------------------------------------------------------------------
gtest_discover_tests(test_database)
