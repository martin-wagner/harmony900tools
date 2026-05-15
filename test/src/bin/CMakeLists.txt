# SPDX-License-Identifier: LGPL-2.1-or-later

# ---------------------------------------------------------------------------
# Source location
# ---------------------------------------------------------------------------
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src/bin)

# ---------------------------------------------------------------------------
# test_data
# ---------------------------------------------------------------------------
add_executable(test_data
    test_timing.cpp
    ${SRC_DIR}/timing.cpp
)

target_include_directories(test_data
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${SRC_DIR}
)

target_link_libraries(test_data
    PRIVATE
        GTest::gtest_main
)



# ---------------------------------------------------------------------------
# add to test suite
# ---------------------------------------------------------------------------
gtest_discover_tests(test_data)


add_subdirectory(irProto)
add_subdirectory(ssIr)
add_subdirectory(learn)