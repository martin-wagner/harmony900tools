# SPDX-License-Identifier: LGPL-2.1-or-later

# ---------------------------------------------------------------------------
# Source location
# ---------------------------------------------------------------------------
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src/cli/learn)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/src/bin)

# ---------------------------------------------------------------------------
# test_single
# ---------------------------------------------------------------------------
add_executable(test_single
    test_trx_single.cpp
    ${LIB_DIR}/data.cpp
    ${SRC_DIR}/trx_base.cpp
    ${SRC_DIR}/trx_single.cpp
)

target_include_directories(test_single
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${SRC_DIR}
)

target_link_libraries(test_single
    PRIVATE
        GTest::gtest_main
)

# ---------------------------------------------------------------------------
# test_stream
# ---------------------------------------------------------------------------
add_executable(test_stream
    test_trx_stream.cpp
    ${SRC_DIR}/trx_base.cpp
    ${SRC_DIR}/trx_stream.cpp
)

target_include_directories(test_stream
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${SRC_DIR}
)

target_link_libraries(test_stream
    PRIVATE
        GTest::gtest_main
)


# ---------------------------------------------------------------------------
# add to test suite
# ---------------------------------------------------------------------------
gtest_discover_tests(test_single)
gtest_discover_tests(test_stream)
