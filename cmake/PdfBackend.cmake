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

    find_package(pdfium QUIET CONFIG)
    if(TARGET pdfium::pdfium)
        message(STATUS "  Found pdfium via CMake config")
    else()
        find_package(pdfium QUIET MODULE)
        if(NOT TARGET pdfium::pdfium)
            message(FATAL_ERROR "Could not find libpdfium. Install it or use a different PDF_BACKEND.")
        endif()
        if(pdfium_PROVIDER)
            message(STATUS "  Found pdfium via ${pdfium_PROVIDER}")
        endif()
    endif()

    target_link_libraries(pdf_backend_iface INTERFACE pdfium::pdfium)

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
        message(STATUS "  Found poppler-qt6 via CMake config")
    else()
        find_package(PopplerQt6 QUIET MODULE)
        if(TARGET Poppler::Qt6)
            if(PopplerQt6_PROVIDER)
                message(STATUS "  Found poppler-qt6 via ${PopplerQt6_PROVIDER}")
            endif()
        elseif(MSVC)
            message(FATAL_ERROR "Could not find poppler-qt6. "
                "Install via vcpkg: vcpkg install poppler[qt6]:x64-windows "
                "then configure cmake with the vcpkg toolchain file.")
        else()
            message(FATAL_ERROR "Could not find poppler-qt6. Install libpoppler-qt6-dev or use a different PDF_BACKEND.")
        endif()
    endif()

    target_link_libraries(pdf_backend_iface INTERFACE Poppler::Qt6)

elseif(PDF_BACKEND STREQUAL "pdfkit")
    message(STATUS "PDF backend: pdfkit (macOS)")
    if(NOT APPLE)
        message(FATAL_ERROR "pdfkit backend is macOS only")
    endif()
    target_compile_definitions(pdf_backend_iface INTERFACE USE_PDFKIT)
    target_link_libraries(pdf_backend_iface INTERFACE "-framework PDFKit")

else()
    message(FATAL_ERROR "Unknown PDF_BACKEND: '${PDF_BACKEND}'. Use: pdfium, poppler, pdfkit, or no_pdf")
endif()
