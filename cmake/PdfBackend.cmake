# PDF backend detection for YACReader
# Creates an INTERFACE target 'pdf_backend_iface' that propagates
# compile definitions and link libraries based on PDF_BACKEND.

include(BackendHelpers)

add_library(pdf_backend_iface INTERFACE)

if(PDF_BACKEND STREQUAL "no_pdf")
    message(STATUS "PDF backend: disabled")
    target_compile_definitions(pdf_backend_iface INTERFACE NO_PDF)

elseif(PDF_BACKEND STREQUAL "pdfium")
    message(STATUS "PDF backend: pdfium")
    target_compile_definitions(pdf_backend_iface INTERFACE USE_PDFIUM)

    if(WIN32)
        yacreader_get_windows_arch_subdir(_pdfium_arch)
        set(_pdfium_include_dir "${CMAKE_SOURCE_DIR}/dependencies/pdfium/win/public")
        set(_pdfium_implib "${CMAKE_SOURCE_DIR}/dependencies/pdfium/win/${_pdfium_arch}/pdfium.lib")
        set(_pdfium_dll "${CMAKE_SOURCE_DIR}/dependencies/pdfium/win/${_pdfium_arch}/pdfium.dll")
        if(NOT EXISTS "${_pdfium_include_dir}/fpdfview.h"
                OR NOT EXISTS "${_pdfium_implib}"
                OR NOT EXISTS "${_pdfium_dll}")
            message(FATAL_ERROR
                "Could not find bundled pdfium for ${_pdfium_arch}. "
                "Expected files under dependencies/pdfium/win/${_pdfium_arch}.")
        endif()

        message(STATUS "  Using bundled pdfium (${_pdfium_arch}, Windows)")
        yacreader_add_imported_library(YACReader::pdfium
            TYPE SHARED
            LOCATION "${_pdfium_dll}"
            IMPORTED_IMPLIB "${_pdfium_implib}"
            INCLUDE_DIR "${_pdfium_include_dir}")
        target_link_libraries(pdf_backend_iface INTERFACE YACReader::pdfium)
    elseif(APPLE)
        set(_pdfium_include_dir "${CMAKE_SOURCE_DIR}/dependencies/pdfium/macx/include")
        set(_pdfium_library "${CMAKE_SOURCE_DIR}/dependencies/pdfium/macx/bin/libpdfium.a")
        if(NOT EXISTS "${_pdfium_include_dir}/fpdfview.h" OR NOT EXISTS "${_pdfium_library}")
            message(FATAL_ERROR "Could not find bundled pdfium under dependencies/pdfium/macx.")
        endif()

        message(STATUS "  Using bundled pdfium (macOS)")
        yacreader_add_imported_library(YACReader::pdfium
            TYPE STATIC
            LOCATION "${_pdfium_library}"
            INCLUDE_DIR "${_pdfium_include_dir}")
        target_link_libraries(pdf_backend_iface INTERFACE YACReader::pdfium)
    else()
        # Linux: try pkg-config first, then a normal CMake search.
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(PDFIUM QUIET IMPORTED_TARGET libpdfium)
        endif()

        if(TARGET PkgConfig::PDFIUM)
            message(STATUS "  Found pdfium via pkg-config")
            target_link_libraries(pdf_backend_iface INTERFACE PkgConfig::PDFIUM)
        else()
            find_path(PDFIUM_INCLUDE_DIR NAMES fpdfview.h PATH_SUFFIXES pdfium)
            find_library(PDFIUM_LIBRARY NAMES pdfium libpdfium)
            if(PDFIUM_INCLUDE_DIR AND PDFIUM_LIBRARY)
                message(STATUS "  Found pdfium via CMake search")
                yacreader_add_imported_library(YACReader::pdfium
                    TYPE UNKNOWN
                    LOCATION "${PDFIUM_LIBRARY}"
                    INCLUDE_DIR "${PDFIUM_INCLUDE_DIR}")
                target_link_libraries(pdf_backend_iface INTERFACE YACReader::pdfium)
            else()
                message(FATAL_ERROR "Could not find libpdfium. Install it or use a different PDF_BACKEND.")
            endif()
        endif()
    endif()

elseif(PDF_BACKEND STREQUAL "poppler")
    message(STATUS "PDF backend: poppler")
    target_compile_definitions(pdf_backend_iface INTERFACE USE_POPPLER)

    if(APPLE)
        message(FATAL_ERROR "Poppler backend is not supported on macOS")
    endif()

    # Try cmake config mode first (poppler built with cmake, or vcpkg with cmake integration)
    # On Windows: vcpkg install poppler[qt6]:x64-windows, then pass the vcpkg toolchain file
    find_package(Poppler QUIET CONFIG)
    if(TARGET Poppler::Qt6)
        message(STATUS "  Found poppler-qt6 via cmake config")
        target_link_libraries(pdf_backend_iface INTERFACE Poppler::Qt6)
    else()
        # Fall back to pkg-config (standard on Linux; on Windows requires vcpkg pkgconf)
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(POPPLER QUIET IMPORTED_TARGET poppler-qt6)
        endif()

        if(TARGET PkgConfig::POPPLER)
            message(STATUS "  Found poppler-qt6 via pkg-config")
            target_link_libraries(pdf_backend_iface INTERFACE PkgConfig::POPPLER)
        elseif(MSVC)
            message(FATAL_ERROR "Could not find poppler-qt6. "
                "Install via vcpkg: vcpkg install poppler[qt6]:x64-windows "
                "then configure cmake with the vcpkg toolchain file.")
        else()
            find_path(POPPLER_QT6_INCLUDE_DIR NAMES poppler-qt6.h PATH_SUFFIXES poppler/qt6)
            find_library(POPPLER_QT6_LIBRARY NAMES poppler-qt6)
            if(POPPLER_QT6_INCLUDE_DIR AND POPPLER_QT6_LIBRARY)
                message(STATUS "  Found poppler-qt6 via CMake search")
                yacreader_add_imported_library(YACReader::poppler_qt6
                    TYPE UNKNOWN
                    LOCATION "${POPPLER_QT6_LIBRARY}"
                    INCLUDE_DIR "${POPPLER_QT6_INCLUDE_DIR}")
                target_link_libraries(pdf_backend_iface INTERFACE YACReader::poppler_qt6)
            else()
                message(FATAL_ERROR "Could not find poppler-qt6. Install libpoppler-qt6-dev or use a different PDF_BACKEND.")
            endif()
        endif()
    endif()

elseif(PDF_BACKEND STREQUAL "pdfkit")
    message(STATUS "PDF backend: pdfkit (macOS)")
    if(NOT APPLE)
        message(FATAL_ERROR "pdfkit backend is macOS only")
    endif()
    target_compile_definitions(pdf_backend_iface INTERFACE USE_PDFKIT)

else()
    message(FATAL_ERROR "Unknown PDF_BACKEND: '${PDF_BACKEND}'. Use: pdfium, poppler, pdfkit, or no_pdf")
endif()

# Set QT_DISABLE_DEPRECATED_BEFORE based on backend
# poppler requires older deprecated API
if(PDF_BACKEND STREQUAL "poppler")
    target_compile_definitions(pdf_backend_iface INTERFACE QT_DISABLE_DEPRECATED_BEFORE=0x050900)
else()
    target_compile_definitions(pdf_backend_iface INTERFACE QT_DISABLE_DEPRECATED_BEFORE=0x050F00)
endif()
