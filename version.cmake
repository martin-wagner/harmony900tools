# SPDX-License-Identifier: LGPL-2.1-or-later

find_package(Git REQUIRED)

# version/host system stuff
set(VERSION_HEADER_OUTPUT  "${CMAKE_BINARY_DIR}/generated/version.h")
set(VERSION_CMAKE_OUTPUT   "${CMAKE_BINARY_DIR}/generated/version.cmake")
 
# Run at configure time so version.h exists before any target needs it.
# Also hooked as a custom target so incremental builds pick up dirty/hash changes.
execute_process(
    COMMAND ${CMAKE_COMMAND}
        -DGIT_EXECUTABLE=${GIT_EXECUTABLE}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DHEADER_TEMPLATE=${CMAKE_SOURCE_DIR}/cmake/version.in.h
        -DHEADER_OUTPUT=${VERSION_HEADER_OUTPUT}
        -DCMAKE_TEMPLATE=${CMAKE_SOURCE_DIR}/cmake/version.in.cmake
        -DCMAKE_OUTPUT=${VERSION_CMAKE_OUTPUT}
        -P ${CMAKE_SOURCE_DIR}/cmake/versionHelper.cmake
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
 
# Re-run on every build so the dirty flag and hash stay fresh.
add_custom_target(version_info ALL
    COMMAND ${CMAKE_COMMAND}
        -DGIT_EXECUTABLE=${GIT_EXECUTABLE}
        -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DHEADER_TEMPLATE=${CMAKE_SOURCE_DIR}/cmake/version.in.h
        -DHEADER_OUTPUT=${VERSION_HEADER_OUTPUT}
        -DCMAKE_TEMPLATE=${CMAKE_SOURCE_DIR}/cmake/version.in.cmake
        -DCMAKE_OUTPUT=${VERSION_CMAKE_OUTPUT}
        -P ${CMAKE_SOURCE_DIR}/cmake/versionHelper.cmake
    COMMENT "Updating version info"
)
 
# Pull in the generated version.cmake so CPack variables are set.
include(${VERSION_CMAKE_OUTPUT})