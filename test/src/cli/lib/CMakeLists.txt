# ---------------------------------------------------------------------------
# Source location
# ---------------------------------------------------------------------------
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src/cli/lib)

# ---------------------------------------------------------------------------
# test_data
# ---------------------------------------------------------------------------
add_executable(test_data
    test_data.cpp
    ${SRC_DIR}/data.cpp
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
