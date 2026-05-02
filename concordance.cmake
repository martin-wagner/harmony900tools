# ===========================================================================
# External dependency versions
# ===========================================================================
set(CONCORDANCE_GIT_URL  "https://github.com/jaymzh/concordance.git")
set(CONCORDANCE_GIT_HASH "9c3d6a06192ce055bfc82f6b6f0be024fa8a7285")

# ===========================================================================
# Paths
# ===========================================================================
# Checked-out source lives here — kept clean, never written to during build.
set(EXTERNAL_SRC_DIR     "${CMAKE_SOURCE_DIR}/src_external/concordance")

# All build artefacts stay inside the build tree.
set(LIBCONCORD_BUILD     "${CMAKE_BINARY_DIR}/libconcord/build")
set(LIBCONCORD_INSTALL   "${CMAKE_BINARY_DIR}/libconcord/install")
set(LIBCONCORD_STAMP     "${CMAKE_BINARY_DIR}/libconcord/stamp")

set(CONCORDANCE_BUILD    "${CMAKE_BINARY_DIR}/concordance_exe/build")
set(CONCORDANCE_INSTALL  "${CMAKE_BINARY_DIR}/concordance_exe/install")
set(CONCORDANCE_STAMP    "${CMAKE_BINARY_DIR}/concordance_exe/stamp")

include(ExternalProject)

# ===========================================================================
# MXE windows cross-compile
# ===========================================================================
if (CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(MXE_BUILD TRUE)
  set(AUTOTOOLS_HOST_FLAG "--host=x86_64-w64-mingw32.shared")
endif()

# ===========================================================================
# Platform-specific library filenames
# ===========================================================================
if (MXE_BUILD)
    set(LIBCONCORD_LINK_LIB  "${LIBCONCORD_INSTALL}/lib/libconcord.dll.a")
    set(LIBCONCORD_BYPRODUCT "${LIBCONCORD_INSTALL}/bin/libconcord-6.dll")
else()
    set(LIBCONCORD_LINK_LIB  "${LIBCONCORD_INSTALL}/lib/libconcord.so.6")
    set(LIBCONCORD_BYPRODUCT "${LIBCONCORD_INSTALL}/lib/libconcord.so.6")
endif()

# ===========================================================================
# Clone step — downloads into src_external/, nothing else.
# We use a dedicated no-op ExternalProject just for the clone so the
# source directory is decoupled from the build directories below.
# ===========================================================================
ExternalProject_Add(concordance_src
    GIT_REPOSITORY  "${CONCORDANCE_GIT_URL}"
    GIT_TAG         "${CONCORDANCE_GIT_HASH}"
    # No shallow clone when pinning a hash — git doesn't allow --depth with
    # an arbitrary commit unless the server supports it; use a full clone.
    GIT_SHALLOW     FALSE

    PREFIX          "${CMAKE_BINARY_DIR}/concordance_src_prefix"
    SOURCE_DIR      "${EXTERNAL_SRC_DIR}"
    STAMP_DIR       "${CMAKE_BINARY_DIR}/concordance_src_prefix/stamp"

    # Nothing to configure, build, or install — clone only.
    CONFIGURE_COMMAND   ""
    BUILD_COMMAND       ""
    INSTALL_COMMAND     ""

    UPDATE_DISCONNECTED TRUE
)

# ===========================================================================
# libconcord — built from the checked-out source, output in build tree.
# ===========================================================================
ExternalProject_Add(libconcord_ext
    DEPENDS         concordance_src

    # No download — source already cloned above.
    DOWNLOAD_COMMAND    ""

    PREFIX          "${CMAKE_BINARY_DIR}/libconcord"
    SOURCE_DIR      "${EXTERNAL_SRC_DIR}"
    BINARY_DIR      "${LIBCONCORD_BUILD}"
    STAMP_DIR       "${LIBCONCORD_STAMP}"

    CONFIGURE_COMMAND
        ${CMAKE_COMMAND} -E make_directory "${EXTERNAL_SRC_DIR}/libconcord/m4"
        COMMAND autoreconf --install "${EXTERNAL_SRC_DIR}/libconcord"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${LIBCONCORD_BUILD}"
        COMMAND "${EXTERNAL_SRC_DIR}/libconcord/configure"
                    --prefix=${LIBCONCORD_INSTALL}
                    --disable-mime-update
                    ${AUTOTOOLS_HOST_FLAG}

    BUILD_COMMAND
        make -C "${LIBCONCORD_BUILD}" -j${cpuCount}

    INSTALL_COMMAND
        make -C "${LIBCONCORD_BUILD}" install

    UPDATE_DISCONNECTED TRUE

    BUILD_BYPRODUCTS "${LIBCONCORD_BYPRODUCT}"
)

# Imported target for use by downstream targets in this CMake project.
if (MXE_BUILD)
  # fixes "IMPORTED_IMPLIB not set" error
  add_library(Libconcord::libconcord UNKNOWN IMPORTED)
else()
  add_library(Libconcord::libconcord SHARED IMPORTED)
endif()
add_dependencies(Libconcord::libconcord libconcord_ext)
set_target_properties(Libconcord::libconcord PROPERTIES
    IMPORTED_LOCATION             "${LIBCONCORD_LINK_LIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${LIBCONCORD_INSTALL}/include"
)
# Include dir doesn't exist at configure time; pre-create it so CMake doesn't
# error on the INTERFACE_INCLUDE_DIRECTORIES property.
file(MAKE_DIRECTORY "${LIBCONCORD_INSTALL}/include")

# ===========================================================================
# concordance executable — optional, excluded from 'make all' / default target.
#
# Build explicitly with:
#   make concordance_exe
# or:
#   cmake --build build --target concordance_exe
# ===========================================================================
ExternalProject_Add(concordance_exe
    DEPENDS         libconcord_ext

    DOWNLOAD_COMMAND    ""

    PREFIX          "${CMAKE_BINARY_DIR}/concordance_exe"
    SOURCE_DIR      "${EXTERNAL_SRC_DIR}"
    BINARY_DIR      "${CONCORDANCE_BUILD}"
    STAMP_DIR       "${CONCORDANCE_STAMP}"

    CONFIGURE_COMMAND
        ${CMAKE_COMMAND} -E make_directory "${EXTERNAL_SRC_DIR}/concordance/m4"
        COMMAND autoreconf --install "${EXTERNAL_SRC_DIR}/concordance"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CONCORDANCE_BUILD}"
        COMMAND "${EXTERNAL_SRC_DIR}/concordance/configure"
                    --prefix=${CONCORDANCE_INSTALL}
                    CFLAGS=-I${LIBCONCORD_INSTALL}/include
                    LDFLAGS=-L${LIBCONCORD_INSTALL}/lib
                    LDFLAGS=-L${LIBCONCORD_INSTALL}/bin
                    ${AUTOTOOLS_HOST_FLAG}

    BUILD_COMMAND
        make -C "${CONCORDANCE_BUILD}" -j${cpuCount}

    INSTALL_COMMAND
        make -C "${CONCORDANCE_BUILD}" install

    UPDATE_DISCONNECTED TRUE

    # Exclude from the default build target (make all).
    EXCLUDE_FROM_ALL TRUE

    BUILD_BYPRODUCTS
        "${CONCORDANCE_INSTALL}/bin/concordance"
)

# ===========================================================================
# clean-external target
# Deletes src_external/. Not hooked into 'make clean' — intentionally manual.
# ===========================================================================
add_custom_target(clean-external
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_SOURCE_DIR}/src_external"
    COMMENT "Removing ${CMAKE_SOURCE_DIR}/src_external"
)

# ===========================================================================
# Usage example
# ===========================================================================
#add_executable(my_program
#    src/main.c
#)
#
#target_link_libraries(my_program PRIVATE Libconcord::libconcord)
#
#set_target_properties(my_program PROPERTIES
#    BUILD_RPATH   "${LIBCONCORD_INSTALL}/lib"
#    INSTALL_RPATH "${LIBCONCORD_INSTALL}/lib"
#)
