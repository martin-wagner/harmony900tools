cmake_minimum_required(VERSION 3.28)

project(LearnTests)

# ---------------------------------------------------------------------------
# Fetch GoogleTest
# ---------------------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.17.0
)

# Don't install gtest alongside the main project
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)
include(GoogleTest)

add_subdirectory(src)
