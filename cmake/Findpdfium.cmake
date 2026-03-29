include(BackendHelpers)
include(FindPackageHandleStandardArgs)

set(pdfium_PROVIDER "")
set(pdfium_TARGET "")
set(pdfium_INCLUDE_DIRS "")
set(pdfium_LIBRARIES "")

if(NOT TARGET pdfium::pdfium AND WIN32)
    yacreader_get_windows_arch_subdir(_pdfium_arch)
    set(_pdfium_include_dir "${CMAKE_SOURCE_DIR}/dependencies/pdfium/win/public")
    set(_pdfium_implib "${CMAKE_SOURCE_DIR}/dependencies/pdfium/win/${_pdfium_arch}/pdfium.lib")
    set(_pdfium_dll "${CMAKE_SOURCE_DIR}/dependencies/pdfium/win/${_pdfium_arch}/pdfium.dll")
    if(EXISTS "${_pdfium_include_dir}/fpdfview.h" AND EXISTS "${_pdfium_implib}" AND EXISTS "${_pdfium_dll}")
        yacreader_add_imported_library(pdfium::pdfium
            TYPE SHARED
            LOCATION "${_pdfium_dll}"
            IMPORTED_IMPLIB "${_pdfium_implib}"
            INCLUDE_DIR "${_pdfium_include_dir}")
        set(pdfium_PROVIDER "bundled dependencies (${_pdfium_arch}, Windows)")
        set(pdfium_INCLUDE_DIRS "${_pdfium_include_dir}")
    endif()
endif()

if(NOT TARGET pdfium::pdfium AND APPLE)
    set(_pdfium_include_dir "${CMAKE_SOURCE_DIR}/dependencies/pdfium/macx/include")
    set(_pdfium_library "${CMAKE_SOURCE_DIR}/dependencies/pdfium/macx/bin/libpdfium.a")
    if(EXISTS "${_pdfium_include_dir}/fpdfview.h" AND EXISTS "${_pdfium_library}")
        yacreader_add_imported_library(pdfium::pdfium
            TYPE STATIC
            LOCATION "${_pdfium_library}"
            INCLUDE_DIR "${_pdfium_include_dir}")
        set(pdfium_PROVIDER "bundled dependencies (macOS)")
        set(pdfium_INCLUDE_DIRS "${_pdfium_include_dir}")
    endif()
endif()

if(NOT TARGET pdfium::pdfium)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(PDFIUM QUIET IMPORTED_TARGET libpdfium)
        if(TARGET PkgConfig::PDFIUM)
            yacreader_add_imported_library(pdfium::pdfium
                TYPE INTERFACE
                LINK_LIBRARIES "PkgConfig::PDFIUM")
            set(pdfium_PROVIDER "pkg-config")
            set(pdfium_INCLUDE_DIRS "${PDFIUM_INCLUDE_DIRS}")
        endif()
    endif()
endif()

if(NOT TARGET pdfium::pdfium)
    find_path(PDFIUM_INCLUDE_DIR NAMES fpdfview.h PATH_SUFFIXES pdfium)
    find_library(PDFIUM_LIBRARY NAMES pdfium libpdfium)
    if(PDFIUM_INCLUDE_DIR AND PDFIUM_LIBRARY)
        yacreader_add_imported_library(pdfium::pdfium
            TYPE UNKNOWN
            LOCATION "${PDFIUM_LIBRARY}"
            INCLUDE_DIR "${PDFIUM_INCLUDE_DIR}")
        set(pdfium_PROVIDER "CMake search")
        set(pdfium_INCLUDE_DIRS "${PDFIUM_INCLUDE_DIR}")
    endif()
endif()

if(TARGET pdfium::pdfium)
    set(pdfium_TARGET pdfium::pdfium)
    set(pdfium_LIBRARIES pdfium::pdfium)
endif()

find_package_handle_standard_args(pdfium REQUIRED_VARS pdfium_TARGET)

mark_as_advanced(PDFIUM_INCLUDE_DIR PDFIUM_LIBRARY)
