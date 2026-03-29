# Comic archive decompression backend (cbx_backend)
# Switched on DECOMPRESSION_BACKEND: unarr | 7zip | libarchive

include(BackendHelpers)

add_library(cbx_backend STATIC)

if(DECOMPRESSION_BACKEND STREQUAL "unarr")
    message(STATUS "Decompression backend: unarr")

    target_sources(cbx_backend PRIVATE
        unarr/compressed_archive.cpp
        unarr/compressed_archive.h
        unarr/extract_delegate.h
    )
    target_include_directories(cbx_backend PUBLIC unarr)
    target_compile_definitions(cbx_backend PUBLIC use_unarr)

    # Try CMake config first, then pkg-config, then bundled/system libraries.
    set(_unarr_target "")

    find_package(unarr QUIET CONFIG)
    if(TARGET unarr::unarr)
        message(STATUS "  Found unarr via CMake config")
        set(_unarr_target unarr::unarr)
    endif()

    if(NOT _unarr_target AND APPLE)
        set(_unarr_include_dir "${CMAKE_SOURCE_DIR}/dependencies/unarr/macx")
        set(_unarr_library "${CMAKE_SOURCE_DIR}/dependencies/unarr/macx/libunarr.a")
        if(EXISTS "${_unarr_include_dir}/unarr.h" AND EXISTS "${_unarr_library}")
            message(STATUS "  Using bundled unarr (macOS)")
            yacreader_add_imported_library(YACReader::unarr
                TYPE STATIC
                LOCATION "${_unarr_library}"
                INCLUDE_DIR "${_unarr_include_dir}"
                LINK_LIBRARIES "z;bz2")
            set(_unarr_target YACReader::unarr)
        endif()
    endif()

    if(NOT _unarr_target AND WIN32)
        yacreader_get_windows_arch_subdir(_unarr_arch)
        set(_unarr_include_dir "${CMAKE_SOURCE_DIR}/dependencies/unarr/win")
        set(_unarr_implib "${CMAKE_SOURCE_DIR}/dependencies/unarr/win/${_unarr_arch}/unarr.lib")
        set(_unarr_dll "${CMAKE_SOURCE_DIR}/dependencies/unarr/win/${_unarr_arch}/unarr.dll")
        if(EXISTS "${_unarr_include_dir}/unarr.h" AND EXISTS "${_unarr_implib}" AND EXISTS "${_unarr_dll}")
            message(STATUS "  Using bundled unarr (${_unarr_arch}, Windows)")
            yacreader_add_imported_library(YACReader::unarr
                TYPE SHARED
                LOCATION "${_unarr_dll}"
                IMPORTED_IMPLIB "${_unarr_implib}"
                INCLUDE_DIR "${_unarr_include_dir}")
            set(_unarr_target YACReader::unarr)
            target_compile_definitions(cbx_backend PRIVATE UNARR_IS_SHARED_LIBRARY)
        endif()
    endif()

    if(NOT _unarr_target)
        find_package(PkgConfig QUIET)
    endif()

    if(NOT _unarr_target AND PkgConfig_FOUND)
        pkg_check_modules(UNARR QUIET IMPORTED_TARGET libunarr)
        if(TARGET PkgConfig::UNARR)
            message(STATUS "  Found unarr via pkg-config")
            set(_unarr_target PkgConfig::UNARR)
        endif()
    endif()

    if(NOT _unarr_target)
        find_path(UNARR_INCLUDE_DIR NAMES unarr.h)
        find_library(UNARR_LIBRARY NAMES unarr libunarr)
        if(UNARR_INCLUDE_DIR AND UNARR_LIBRARY)
            message(STATUS "  Found unarr via CMake search")
            yacreader_add_imported_library(YACReader::unarr
                TYPE UNKNOWN
                LOCATION "${UNARR_LIBRARY}"
                INCLUDE_DIR "${UNARR_INCLUDE_DIR}")
            set(_unarr_target YACReader::unarr)
        endif()
    endif()

    if(NOT _unarr_target)
        message(FATAL_ERROR
            "Could not find unarr. Install libunarr or use a different DECOMPRESSION_BACKEND.")
    endif()

    target_link_libraries(cbx_backend PRIVATE ${_unarr_target})

elseif(DECOMPRESSION_BACKEND STREQUAL "7zip")
    message(STATUS "Decompression backend: 7zip (in-tree)")

    # Auto-download 7zip source if not present
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib7zip/CPP")
        include(FetchContent)
        FetchContent_Declare(
            lib7zip
            URL "https://github.com/YACReader/yacreader-7z-deps/raw/main/7z2301-src.7z"
            SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib7zip"
            DOWNLOAD_EXTRACT_TIMESTAMP ON
        )
        message(STATUS "  Downloading 7zip source from yacreader-7z-deps...")
        FetchContent_MakeAvailable(lib7zip)
    endif()

    target_sources(cbx_backend PRIVATE
        compressed_archive.cpp
        compressed_archive.h
        extract_delegate.h
        7z_includes.h
        open_callbacks.h
        extract_callbacks.h
        lib7zip/CPP/Windows/FileIO.cpp
        lib7zip/CPP/Windows/PropVariant.cpp
        lib7zip/CPP/Windows/PropVariantConv.cpp
        lib7zip/CPP/Common/IntToString.cpp
        lib7zip/CPP/Common/MyString.cpp
        lib7zip/CPP/Common/MyVector.cpp
        lib7zip/CPP/Common/Wildcard.cpp
        lib7zip/CPP/7zip/Common/FileStreams.cpp
        lib7zip/C/Alloc.c
        lib7zip/CPP/7zip/Common/StreamObjects.cpp
    )

    if(UNIX)
        target_sources(cbx_backend PRIVATE
            lib7zip/CPP/Common/NewHandler.cpp
            lib7zip/CPP/Windows/DLL.cpp
            lib7zip/CPP/Windows/FileDir.cpp
            lib7zip/CPP/Windows/FileFind.cpp
            lib7zip/CPP/Windows/FileName.cpp
            lib7zip/CPP/Windows/TimeUtils.cpp
            lib7zip/CPP/Common/UTFConvert.cpp
            lib7zip/CPP/Common/MyWindows.cpp
            lib7zip/CPP/Common/StringConvert.cpp
        )
    endif()

    target_include_directories(cbx_backend
        PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE lib7zip/CPP
    )

    if(APPLE)
        target_compile_definitions(cbx_backend PRIVATE
            _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE NDEBUG _REENTRANT
            ENV_UNIX _7ZIP_LARGE_PAGES ENV_MACOSX _TCHAR_DEFINED
            UNICODE _UNICODE UNIX_USE_WIN_FILE)
        target_link_libraries(cbx_backend PRIVATE
            "-framework IOKit" "-framework CoreFoundation")
    elseif(UNIX)
        target_compile_definitions(cbx_backend PRIVATE
            _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE NDEBUG _REENTRANT
            ENV_UNIX _7ZIP_LARGE_PAGES UNICODE _UNICODE UNIX_USE_WIN_FILE)
    endif()

elseif(DECOMPRESSION_BACKEND STREQUAL "libarchive")
    message(STATUS "Decompression backend: libarchive")

    if(WIN32 OR APPLE)
        message(FATAL_ERROR "libarchive backend is not supported on Windows or macOS")
    endif()

    target_sources(cbx_backend PRIVATE
        libarchive/compressed_archive.cpp
        libarchive/compressed_archive.h
        libarchive/extract_delegate.h
    )
    target_include_directories(cbx_backend PUBLIC libarchive)
    target_compile_definitions(cbx_backend PUBLIC use_libarchive)

    find_package(LibArchive REQUIRED)
    message(STATUS "  Found libarchive ${LibArchive_VERSION}")
    target_link_libraries(cbx_backend PRIVATE LibArchive::LibArchive)

else()
    message(FATAL_ERROR
        "Unknown DECOMPRESSION_BACKEND: '${DECOMPRESSION_BACKEND}'. "
        "Use: unarr, 7zip, or libarchive")
endif()

# Qt, yr_global, and QsLog are needed by all backends
target_link_libraries(cbx_backend PRIVATE Qt::Core yr_global QsLog)
