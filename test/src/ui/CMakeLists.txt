# SPDX-License-Identifier: LGPL-2.1-or-later

# ---------------------------------------------------------------------------
# Qt — add this if not already in a parent CMakeLists.txt
# ---------------------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Core Gui  Widgets Test)  

# ---------------------------------------------------------------------------
# Source location — adjust to match your tree
# ---------------------------------------------------------------------------
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src/ui)  

# ---------------------------------------------------------------------------
# test_settings
# ---------------------------------------------------------------------------
add_executable(test_settings
    test_settings.cpp
    ${SRC_DIR}/settings.cpp
)

# MOC must run on settings.h (Q_OBJECT macro).
set_target_properties(test_settings PROPERTIES AUTOMOC ON)

target_include_directories(test_settings
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src   
        ${SRC_DIR}
)

target_link_libraries(test_settings
    PRIVATE
        GTest::gtest          # NOT gtest_main — we supply our own main()
        Qt6::Widgets
        Qt6::Test             # QSignalSpy lives here
)

# ---------------------------------------------------------------------------
# add to test suite
# ---------------------------------------------------------------------------
gtest_discover_tests(test_settings DISCOVERY_MODE PRE_TEST) #add DISCOVERY_MODE PRE_TEST to at least one test to force cmake to scan for newly added tests. bug?