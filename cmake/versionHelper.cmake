# SPDX-License-Identifier: LGPL-2.1-or-later

# git_version.cmake
# Called via cmake -P or add_custom_target at build time.
# Required input variables (pass with -D):
#   GIT_EXECUTABLE   - path to git
#   SOURCE_DIR       - project root (where .git lives)
#   HEADER_TEMPLATE  - path to version.h.in
#   HEADER_OUTPUT    - path to generated version.h
#   CMAKE_TEMPLATE   - path to version.cmake.in
#   CMAKE_OUTPUT     - path to generated version.cmake

# ---------- git describe ----------
execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags --long --dirty
    WORKING_DIRECTORY ${SOURCE_DIR}
    OUTPUT_VARIABLE GIT_DESCRIBE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE GIT_DESCRIBE_RESULT
)

if (NOT GIT_DESCRIBE_RESULT EQUAL 0)
    set(GIT_DESCRIBE "v0.0-0-g0000000")
    message(WARNING "git describe failed, using fallback: ${GIT_DESCRIBE}")
endif()

# ---------- parse describe output ----------
# Expected format: v<major>.<minor>-<patch>-g<hash>[-dirty]
string(REGEX MATCH "^v?([0-9]+)\\.([0-9]+)-([0-9]+)-g([0-9a-f]+)(-dirty)?$"
       _match "${GIT_DESCRIBE}")

if (NOT _match)
    message(WARNING "Could not parse git describe output: '${GIT_DESCRIBE}'")
    set(VERSION_MAJOR 0)
    set(VERSION_MINOR 0)
    set(VERSION_PATCH 0)
    set(GIT_HASH      "0000000")
    set(GIT_DIRTY     0)
else()
    set(VERSION_MAJOR ${CMAKE_MATCH_1})
    set(VERSION_MINOR ${CMAKE_MATCH_2})
    set(VERSION_PATCH ${CMAKE_MATCH_3})
    set(GIT_HASH      ${CMAKE_MATCH_4})
    if (CMAKE_MATCH_5)
        set(GIT_DIRTY 1)
    else()
        set(GIT_DIRTY 0)
    endif()
endif()

set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
if (GIT_DIRTY)
    set(VERSION_FULL  "${VERSION_STRING}-${GIT_HASH}-dirty")
else()
    set(VERSION_FULL  "${VERSION_STRING}-${GIT_HASH}")
endif()

# ---------- compiler version ----------
execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} --version
    OUTPUT_VARIABLE COMPILER_VERSION_RAW
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
string(REGEX MATCH "^[^\n]+" COMPILER_VERSION "${COMPILER_VERSION_RAW}")

# ---------- username ----------
if (DEFINED ENV{USER})
    set(BUILD_USER "$ENV{USER}")
elseif (DEFINED ENV{USERNAME})
    set(BUILD_USER "$ENV{USERNAME}")
else()
    set(BUILD_USER "unknown")
endif()

# ---------- host OS ----------
if (EXISTS "/etc/os-release")
    file(READ "/etc/os-release" OS_RELEASE_CONTENT)
    string(REGEX MATCH "PRETTY_NAME=\"([^\"]*)\"" _os_match "${OS_RELEASE_CONTENT}")
    if (_os_match)
        set(HOST_OS "${CMAKE_MATCH_1}")
    else()
        set(HOST_OS "Linux (unknown distro)")
    endif()
elseif (CMAKE_HOST_SYSTEM_NAME)
    set(HOST_OS "${CMAKE_HOST_SYSTEM_NAME}")
else()
    set(HOST_OS "unknown")
endif()

# ---------- generate files ----------
configure_file(${HEADER_TEMPLATE} ${HEADER_OUTPUT} @ONLY)
configure_file(${CMAKE_TEMPLATE}  ${CMAKE_OUTPUT}  @ONLY)

message(STATUS "Version: ${VERSION_FULL}  dirty=${GIT_DIRTY}  os=${HOST_OS}  user=${BUILD_USER}")