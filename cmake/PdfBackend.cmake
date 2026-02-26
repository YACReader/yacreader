# PDF backend detection for YACReader
# Creates an INTERFACE target 'pdf_backend_iface' that propagates
# compile definitions and link libraries based on PDF_BACKEND.

add_library(pdf_backend_iface INTERFACE)

if(PDF_BACKEND STREQUAL "no_pdf")
    message(STATUS "PDF backend: disabled")
    target_compile_definitions(pdf_backend_iface INTERFACE NO_PDF)

elseif(PDF_BACKEND STREQUAL "pdfium")
    message(STATUS "PDF backend: pdfium")
    target_compile_definitions(pdf_backend_iface INTERFACE USE_PDFIUM)

    if(MSVC)
        target_include_directories(pdf_backend_iface INTERFACE
            "${CMAKE_SOURCE_DIR}/dependencies/pdfium/win/public")
        if(CMAKE_SYSTEM_PROCESSOR STREQUAL "ARM64" OR CMAKE_CXX_COMPILER_ARCHITECTURE_ID STREQUAL "ARM64")
            target_link_directories(pdf_backend_iface INTERFACE
                "${CMAKE_SOURCE_DIR}/dependencies/pdfium/win/arm64")
        else()
            target_link_directories(pdf_backend_iface INTERFACE
                "${CMAKE_SOURCE_DIR}/dependencies/pdfium/win/x64")
        endif()
        target_link_libraries(pdf_backend_iface INTERFACE pdfium)
    elseif(APPLE)
        target_include_directories(pdf_backend_iface INTERFACE
            "${CMAKE_SOURCE_DIR}/dependencies/pdfium/macx/include")
        target_link_directories(pdf_backend_iface INTERFACE
            "${CMAKE_SOURCE_DIR}/dependencies/pdfium/macx/bin")
        target_link_libraries(pdf_backend_iface INTERFACE pdfium)
    else()
        # Linux: try pkg-config first, then system path
        pkg_check_modules(PDFIUM QUIET IMPORTED_TARGET libpdfium)
        if(PDFIUM_FOUND)
            message(STATUS "  Found pdfium via pkg-config")
            target_link_libraries(pdf_backend_iface INTERFACE PkgConfig::PDFIUM)
        elseif(EXISTS "/usr/include/pdfium")
            message(STATUS "  Found pdfium at /usr/include/pdfium")
            target_include_directories(pdf_backend_iface INTERFACE /usr/include/pdfium)
            target_link_libraries(pdf_backend_iface INTERFACE pdfium)
        else()
            message(FATAL_ERROR "Could not find libpdfium. Install it or use a different PDF_BACKEND.")
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
        pkg_check_modules(POPPLER QUIET IMPORTED_TARGET poppler-qt6)
        if(POPPLER_FOUND)
            message(STATUS "  Found poppler-qt6 via pkg-config")
            target_link_libraries(pdf_backend_iface INTERFACE PkgConfig::POPPLER)
        elseif(NOT MSVC AND EXISTS "/usr/include/poppler/qt6")
            message(STATUS "  Found poppler-qt6 at /usr/include/poppler/qt6")
            target_include_directories(pdf_backend_iface INTERFACE /usr/include/poppler/qt6)
            target_link_libraries(pdf_backend_iface INTERFACE poppler-qt6)
        elseif(MSVC)
            message(FATAL_ERROR "Could not find poppler-qt6. "
                "Install via vcpkg: vcpkg install poppler[qt6]:x64-windows "
                "then configure cmake with the vcpkg toolchain file.")
        else()
            message(FATAL_ERROR "Could not find poppler-qt6. Install libpoppler-qt6-dev or use a different PDF_BACKEND.")
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
