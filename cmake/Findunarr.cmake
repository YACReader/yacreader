include(BackendHelpers)
include(FindPackageHandleStandardArgs)

set(unarr_PROVIDER "")
set(unarr_TARGET "")
set(unarr_INCLUDE_DIRS "")
set(unarr_LIBRARIES "")

get_filename_component(_unarr_bundled_root "${CMAKE_CURRENT_LIST_DIR}/../dependencies/unarr" ABSOLUTE)

if(NOT TARGET unarr::unarr AND APPLE)
    set(_unarr_include_dir "${_unarr_bundled_root}/macx")
    set(_unarr_library "${_unarr_bundled_root}/macx/libunarr.a")
    if(EXISTS "${_unarr_include_dir}/unarr.h" AND EXISTS "${_unarr_library}")
        yacreader_add_imported_library(unarr::unarr
            TYPE STATIC
            LOCATION "${_unarr_library}"
            INCLUDE_DIR "${_unarr_include_dir}"
            LINK_LIBRARIES "z;bz2")
        set(unarr_PROVIDER "bundled dependencies (macOS)")
        set(unarr_INCLUDE_DIRS "${_unarr_include_dir}")
    endif()
endif()

if(NOT TARGET unarr::unarr AND WIN32)
    yacreader_get_windows_arch_subdir(_unarr_arch)
    set(_unarr_include_dir "${_unarr_bundled_root}/win")
    set(_unarr_implib "${_unarr_bundled_root}/win/${_unarr_arch}/unarr.lib")
    set(_unarr_dll "${_unarr_bundled_root}/win/${_unarr_arch}/unarr.dll")
    if(EXISTS "${_unarr_include_dir}/unarr.h" AND EXISTS "${_unarr_implib}" AND EXISTS "${_unarr_dll}")
        yacreader_add_imported_library(unarr::unarr
            TYPE SHARED
            LOCATION "${_unarr_dll}"
            IMPORTED_IMPLIB "${_unarr_implib}"
            INCLUDE_DIR "${_unarr_include_dir}"
            COMPILE_DEFINITIONS "UNARR_IS_SHARED_LIBRARY")
        set(unarr_PROVIDER "bundled dependencies (${_unarr_arch}, Windows)")
        set(unarr_INCLUDE_DIRS "${_unarr_include_dir}")
    endif()
endif()

if(NOT TARGET unarr::unarr)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(UNARR QUIET IMPORTED_TARGET libunarr)
        if(TARGET PkgConfig::UNARR)
            yacreader_add_imported_library(unarr::unarr
                TYPE INTERFACE
                LINK_LIBRARIES "PkgConfig::UNARR")
            set(unarr_PROVIDER "pkg-config")
            set(unarr_INCLUDE_DIRS "${UNARR_INCLUDE_DIRS}")
        endif()
    endif()
endif()

if(NOT TARGET unarr::unarr)
    find_path(UNARR_INCLUDE_DIR NAMES unarr.h)
    find_library(UNARR_LIBRARY NAMES unarr libunarr)
    if(UNARR_INCLUDE_DIR AND UNARR_LIBRARY)
        yacreader_add_imported_library(unarr::unarr
            TYPE UNKNOWN
            LOCATION "${UNARR_LIBRARY}"
            INCLUDE_DIR "${UNARR_INCLUDE_DIR}")
        set(unarr_PROVIDER "CMake search")
        set(unarr_INCLUDE_DIRS "${UNARR_INCLUDE_DIR}")
    endif()
endif()

if(TARGET unarr::unarr)
    set(unarr_TARGET unarr::unarr)
    set(unarr_LIBRARIES unarr::unarr)
endif()

find_package_handle_standard_args(unarr REQUIRED_VARS unarr_TARGET)

mark_as_advanced(UNARR_INCLUDE_DIR UNARR_LIBRARY)
