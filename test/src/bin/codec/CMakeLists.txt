# SPDX-License-Identifier: LGPL-2.1-or-later

# ---------------------------------------------------------------------------
# Qt — add this if not already in a parent CMakeLists.txt
# ---------------------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Core)  # QString / QList

# ---------------------------------------------------------------------------
# Source location
# ---------------------------------------------------------------------------
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src/bin/codec)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/src/bin)

# ---------------------------------------------------------------------------
# test_irProto
# ---------------------------------------------------------------------------
add_executable(test_codec
    test_decode.cpp
    test_encode.cpp
    ${SRC_DIR}/decode.cpp
    ${SRC_DIR}/encode.cpp
    ${LIB_DIR}/timing.cpp
    ${LIB_DIR}/irProto/code.cpp
    ${CMAKE_SOURCE_DIR}/src/document/data/enum.cpp
)

target_include_directories(test_codec
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${SRC_DIR}
)

target_link_libraries(test_codec
    PRIVATE
        GTest::gtest_main
        magic_enum::magic_enum
        IRremoteESP8266
        Qt6::Core
)

# Inject paths for test files
#target_compile_definitions(test_irProto PRIVATE
#)

# ---------------------------------------------------------------------------
# add to test suite
# ---------------------------------------------------------------------------
gtest_discover_tests(test_codec)
