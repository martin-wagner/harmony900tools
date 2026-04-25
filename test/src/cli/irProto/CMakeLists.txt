# ---------------------------------------------------------------------------
# Source location
# ---------------------------------------------------------------------------
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src/cli/irProto)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/src/cli/lib)

# ---------------------------------------------------------------------------
# test_irProto
# ---------------------------------------------------------------------------
add_executable(test_irProto
    test_file.cpp
    ${LIB_DIR}/data.cpp
    ${SRC_DIR}/file.cpp
)

target_include_directories(test_irProto
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${SRC_DIR}
)

target_link_libraries(test_irProto
    PRIVATE
        GTest::gtest_main
)

# Inject path to the real irProto.bin so the fixture can find it
target_compile_definitions(test_irProto PRIVATE
    REAL_irProto_BIN_PATH="${CMAKE_CURRENT_SOURCE_DIR}/irProto.bin"
)


# ---------------------------------------------------------------------------
# add to test suite
# ---------------------------------------------------------------------------
gtest_discover_tests(test_irProto)
