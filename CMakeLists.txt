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
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Compiler options (MSVC flags, NOMINMAX, etc.)
include(cmake/CompilerOptions.cmake)

# --- Build options ---

# Build configuration
option(BUILD_TESTS "Build tests" ON)
option(BUILD_SERVER_STANDALONE "Server standalone install (Linux only)" OFF)

# Build number (set by CI)
set(BUILD_NUMBER "" CACHE STRING "CI build number")

set(YACREADER_DECOMPRESSION_BACKENDS unarr 7zip libarchive)
set(YACREADER_PDF_BACKENDS pdfium poppler pdfkit no_pdf)

# --- Platform defaults (mirrors config.pri) ---
if(UNIX AND NOT APPLE)
    set(_default_decompression_backend "unarr")
    set(_default_pdf_backend "poppler")
elseif(APPLE)
    set(_default_decompression_backend "7zip")
    set(_default_pdf_backend "pdfkit")
else()
    set(_default_decompression_backend "7zip")
    set(_default_pdf_backend "pdfium")
endif()

# Archive decompression backend (mutually exclusive)
set(_decompression_backend_value "")
if(DEFINED CACHE{DECOMPRESSION_BACKEND})
    set(_decompression_backend_value "$CACHE{DECOMPRESSION_BACKEND}")
endif()
if(_decompression_backend_value STREQUAL "")
    set(_decompression_backend_value "${_default_decompression_backend}")
    string(JOIN ", " _available_decompression_backends ${YACREADER_DECOMPRESSION_BACKENDS})
    message(STATUS
        "DECOMPRESSION_BACKEND not set, defaulting to ${_decompression_backend_value}. "
        "Available: ${_available_decompression_backends}.")
endif()
set(DECOMPRESSION_BACKEND "${_decompression_backend_value}" CACHE STRING
    "Archive backend: unarr | 7zip | libarchive" FORCE)
set_property(CACHE DECOMPRESSION_BACKEND PROPERTY STRINGS ${YACREADER_DECOMPRESSION_BACKENDS})
if(NOT DECOMPRESSION_BACKEND IN_LIST YACREADER_DECOMPRESSION_BACKENDS)
    string(JOIN ", " _valid_decompression_backends ${YACREADER_DECOMPRESSION_BACKENDS})
    message(FATAL_ERROR
        "Unknown DECOMPRESSION_BACKEND: '${DECOMPRESSION_BACKEND}'. "
        "Use one of: ${_valid_decompression_backends}")
endif()

# PDF rendering backend (mutually exclusive)
set(_pdf_backend_value "")
if(DEFINED CACHE{PDF_BACKEND})
    set(_pdf_backend_value "$CACHE{PDF_BACKEND}")
endif()
if(_pdf_backend_value STREQUAL "")
    set(_pdf_backend_value "${_default_pdf_backend}")
    string(JOIN ", " _available_pdf_backends ${YACREADER_PDF_BACKENDS})
    message(STATUS
        "PDF_BACKEND not set, defaulting to ${_pdf_backend_value}. "
        "Available: ${_available_pdf_backends}.")
endif()
set(PDF_BACKEND "${_pdf_backend_value}" CACHE STRING
    "PDF backend: pdfium | poppler | pdfkit | no_pdf" FORCE)
set_property(CACHE PDF_BACKEND PROPERTY STRINGS ${YACREADER_PDF_BACKENDS})
if(NOT PDF_BACKEND IN_LIST YACREADER_PDF_BACKENDS)
    string(JOIN ", " _valid_pdf_backends ${YACREADER_PDF_BACKENDS})
    message(FATAL_ERROR
        "Unknown PDF_BACKEND: '${PDF_BACKEND}'. "
        "Use one of: ${_valid_pdf_backends}")
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
        GuiPrivate
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
        TextToSpeech
        Widgets
    )
endif()
qt_standard_project_setup()

# PDF backend detection (creates pdf_backend_iface INTERFACE target)
include(PdfBackend)

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
    add_subdirectory(image_processing)
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
