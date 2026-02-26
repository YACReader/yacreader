cmake_minimum_required(VERSION 3.25)

project(YACReader
    VERSION 10.0.0
    LANGUAGES C CXX
)

# Enable Objective-C/C++ on Apple platforms
if(APPLE)
    enable_language(OBJC)
    enable_language(OBJCXX)
endif()

# Enforce out-of-source build
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" _in_source_check)
if(EXISTS "${_in_source_check}")
    message(FATAL_ERROR "In-source builds are not allowed. Use: cmake -B build")
endif()

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default build type (single-config generators only)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Install paths
include(GNUInstallDirs)
find_package(PkgConfig)

# Compiler options (MSVC flags, NOMINMAX, etc.)
include(cmake/CompilerOptions.cmake)

# --- Build options ---

# Archive decompression backend (mutually exclusive)
set(DECOMPRESSION_BACKEND "" CACHE STRING "Archive backend: unarr | 7zip | libarchive")
set_property(CACHE DECOMPRESSION_BACKEND PROPERTY STRINGS "unarr" "7zip" "libarchive")

# PDF rendering backend (mutually exclusive)
set(PDF_BACKEND "" CACHE STRING "PDF backend: pdfium | poppler | pdfkit | no_pdf")
set_property(CACHE PDF_BACKEND PROPERTY STRINGS "pdfium" "poppler" "pdfkit" "no_pdf")

# Build configuration
option(BUILD_TESTS "Build tests" ON)
option(BUILD_SERVER_STANDALONE "Server standalone install (Linux only)" OFF)

# Build number (set by CI)
set(BUILD_NUMBER "" CACHE STRING "CI build number")

# --- Platform defaults (mirrors config.pri) ---
if(NOT DECOMPRESSION_BACKEND OR DECOMPRESSION_BACKEND STREQUAL "")
    if(UNIX AND NOT APPLE)
        set(DECOMPRESSION_BACKEND "unarr")
    else()
        set(DECOMPRESSION_BACKEND "7zip")
    endif()
    message(STATUS "DECOMPRESSION_BACKEND defaulted to: ${DECOMPRESSION_BACKEND}")
endif()

if(NOT PDF_BACKEND OR PDF_BACKEND STREQUAL "")
    if(UNIX AND NOT APPLE)
        set(PDF_BACKEND "poppler")
    elseif(APPLE)
        set(PDF_BACKEND "pdfkit")
    else()
        set(PDF_BACKEND "pdfium")
    endif()
    message(STATUS "PDF_BACKEND defaulted to: ${PDF_BACKEND}")
endif()

# macOS universal binary default
if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Target architectures")
endif()

# --- Qt ---
# BUILD_SERVER_STANDALONE only needs Qt 6.4+ (no GUI/shader components required)
if(BUILD_SERVER_STANDALONE)
    find_package(Qt6 6.4 REQUIRED COMPONENTS
        Core
        Core5Compat
        Gui
        LinguistTools
        Network
        Sql
    )
else()
    find_package(Qt6 6.7 REQUIRED COMPONENTS
        Core
        Core5Compat
        Gui
        LinguistTools
        Multimedia
        Network
        OpenGLWidgets
        Quick
        QuickControls2
        QuickWidgets
        Qml
        ShaderTools
        Sql
        Svg
        Test
        Widgets
    )
endif()
qt_standard_project_setup()

# PDF backend detection (creates pdf_backend_iface INTERFACE target)
include(cmake/PdfBackend.cmake)

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Build number define
if(BUILD_NUMBER)
    add_compile_definitions("BUILD_NUMBER=\"${BUILD_NUMBER}\"")
endif()

# --- Subdirectories (dependency order) ---
add_subdirectory(third_party)
add_subdirectory(compressed_archive)
add_subdirectory(common)

# GUI-only subdirectories (not needed for server-standalone builds)
if(NOT BUILD_SERVER_STANDALONE)
    add_subdirectory(shortcuts_management)
    add_subdirectory(custom_widgets)
endif()

add_subdirectory(YACReaderLibrary/server)

if(NOT BUILD_SERVER_STANDALONE)
    add_subdirectory(YACReaderLibrary/comic_vine)
endif()

# Always add YACReaderLibrary: defines library_common and db_helper (shared with server)
# When BUILD_SERVER_STANDALONE=ON, only those shared targets are built (not the app)
add_subdirectory(YACReaderLibrary)

if(NOT BUILD_SERVER_STANDALONE)
    add_subdirectory(YACReader)
endif()

add_subdirectory(YACReaderLibraryServer)

if(BUILD_TESTS AND NOT BUILD_SERVER_STANDALONE)
    enable_testing()
    add_subdirectory(tests)
endif()

# --- Linux top-level install rules ---
if(UNIX AND NOT APPLE)
    # Man pages
    if(EXISTS "${CMAKE_SOURCE_DIR}/YACReader.1")
        install(FILES YACReader.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
    endif()
    if(EXISTS "${CMAKE_SOURCE_DIR}/YACReaderLibrary.1")
        install(FILES YACReaderLibrary.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
    endif()

    # Desktop files
    if(EXISTS "${CMAKE_SOURCE_DIR}/YACReader.desktop")
        install(FILES YACReader.desktop YACReaderLibrary.desktop
            DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    endif()

    # Icons
    if(EXISTS "${CMAKE_SOURCE_DIR}/YACReader.svg")
        install(FILES YACReader.svg YACReaderLibrary.svg
            DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
    endif()
endif()

# Summary
message(STATUS "")
message(STATUS "YACReader ${PROJECT_VERSION} build configuration:")
message(STATUS "  Decompression backend: ${DECOMPRESSION_BACKEND}")
message(STATUS "  PDF backend:           ${PDF_BACKEND}")
message(STATUS "  Build tests:           ${BUILD_TESTS}")
message(STATUS "  Server standalone:     ${BUILD_SERVER_STANDALONE}")
message(STATUS "  Build type:            ${CMAKE_BUILD_TYPE}")
if(BUILD_NUMBER)
    message(STATUS "  Build number:          ${BUILD_NUMBER}")
endif()
message(STATUS "")
