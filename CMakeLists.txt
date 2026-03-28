cmake_minimum_required(VERSION 3.28)

project(
  harmony900
  VERSION 0.1
  LANGUAGES CXX
)
set(CXX_STANDARD 23)
enable_testing()

add_subdirectory(src)
add_subdirectory(test)