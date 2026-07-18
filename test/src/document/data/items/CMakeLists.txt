# SPDX-License-Identifier: LGPL-2.1-or-later

# ---------------------------------------------------------------------------
# Source locations
# ---------------------------------------------------------------------------
set(SERIAL_DIR ${SRC_DIR}/items)
set(ITEMS_DIR ${SRC_DIR}/items)

# ---------------------------------------------------------------------------
# test_deviceJson
# ---------------------------------------------------------------------------
add_executable(test_deviceJson
    test_deviceJson.cpp
    ${SERIAL_DIR}/deviceJson.cpp
    ${SRC_DIR}/enum.cpp
)

target_include_directories(test_deviceJson
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${SRC_DIR}
        ${SERIAL_DIR}
        ${ITEMS_DIR}
)

target_link_libraries(test_deviceJson
    PRIVATE
        GTest::gtest_main
        magic_enum::magic_enum
        nlohmann_json::nlohmann_json
        Qt6::Core
)

# ---------------------------------------------------------------------------
# test_commonJson (UserInfo, ControllerInfo)
# ---------------------------------------------------------------------------
add_executable(test_commonJson
    test_commonJson.cpp
    ${SERIAL_DIR}/commonJson.cpp
    ${SRC_DIR}/enum.cpp
)

target_include_directories(test_commonJson
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${SRC_DIR}
        ${SERIAL_DIR}
        ${ITEMS_DIR}
)

target_link_libraries(test_commonJson
    PRIVATE
        GTest::gtest_main
        magic_enum::magic_enum
        nlohmann_json::nlohmann_json
        Qt6::Core
)

# ---------------------------------------------------------------------------
# test_activityJson (Activity, Channel, Role)
# depends on deviceJson.cpp too: Activity buttons reuse Button/ButtonType
# (de)serialization defined there.
# ---------------------------------------------------------------------------
add_executable(test_activityJson
    test_activityJson.cpp
    ${SERIAL_DIR}/activityJson.cpp
    ${SERIAL_DIR}/deviceJson.cpp
    ${SRC_DIR}/enum.cpp
)

target_include_directories(test_activityJson
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${SRC_DIR}
        ${SERIAL_DIR}
        ${ITEMS_DIR}
)

target_link_libraries(test_activityJson
    PRIVATE
        GTest::gtest_main
        magic_enum::magic_enum
        nlohmann_json::nlohmann_json
        Qt6::Core
)

# ---------------------------------------------------------------------------
# add to test suite
# ---------------------------------------------------------------------------
gtest_discover_tests(test_deviceJson)
gtest_discover_tests(test_commonJson)
gtest_discover_tests(test_activityJson)
