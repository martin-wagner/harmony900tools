cmake_minimum_required(VERSION 3.28)

project(
  harmony900
  VERSION 0.1
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall)

set(INCLUDE_UNIT_TESTING OFF)

# Check build type case-insensitively
string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_LOWER)
if(BUILD_TYPE_LOWER STREQUAL "test")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
  set(INCLUDE_UNIT_TESTING ON)
  enable_testing()
endif()

# cpu count "-j"
include(ProcessorCount)
ProcessorCount(cpuCount)

# lto
include(CheckIPOSupported)
check_ipo_supported(RESULT ltoSupported OUTPUT error)
if(ltoSupported AND BUILD_TYPE_LOWER STREQUAL "release")
    message(STATUS "IPO / LTO enabled")
    set(ltoEnabled TRUE)
endif()

# add libs
include(concordance.cmake)

# add project
add_subdirectory(src)

if(INCLUDE_UNIT_TESTING)
  add_subdirectory(test)
endif()
