# Comic archive decompression backend (cbx_backend)
# Switched on DECOMPRESSION_BACKEND: unarr | 7zip | libarchive

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

    # Try CMake config first, then pkg-config, then system path
    find_package(unarr QUIET)
    if(unarr_FOUND)
        message(STATUS "  Found unarr via CMake config")
        target_link_libraries(cbx_backend PRIVATE unarr::unarr)
    else()
        pkg_check_modules(UNARR QUIET IMPORTED_TARGET libunarr)
        if(UNARR_FOUND)
            message(STATUS "  Found unarr via pkg-config")
            target_link_libraries(cbx_backend PRIVATE PkgConfig::UNARR)
        elseif(APPLE AND EXISTS "${CMAKE_SOURCE_DIR}/dependencies/unarr/macx/libunarr.a")
            message(STATUS "  Found prebuilt unarr in dependencies (macOS)")
            target_include_directories(cbx_backend PRIVATE
                "${CMAKE_SOURCE_DIR}/dependencies/unarr/macx")
            target_link_directories(cbx_backend PRIVATE
                "${CMAKE_SOURCE_DIR}/dependencies/unarr/macx")
            target_link_libraries(cbx_backend PRIVATE unarr z bz2)
        elseif(WIN32 AND EXISTS "${CMAKE_SOURCE_DIR}/dependencies/unarr/win/unarr.h")
            message(STATUS "  Found prebuilt unarr in dependencies (Windows)")
            target_include_directories(cbx_backend PRIVATE
                "${CMAKE_SOURCE_DIR}/dependencies/unarr/win")
            if(CMAKE_SYSTEM_PROCESSOR STREQUAL "ARM64")
                target_link_directories(cbx_backend PRIVATE
                    "${CMAKE_SOURCE_DIR}/dependencies/unarr/win/arm64")
            else()
                target_link_directories(cbx_backend PRIVATE
                    "${CMAKE_SOURCE_DIR}/dependencies/unarr/win/x64")
            endif()
            target_link_libraries(cbx_backend PRIVATE unarr)
            target_compile_definitions(cbx_backend PRIVATE UNARR_IS_SHARED_LIBRARY)
        elseif(EXISTS "/usr/include/unarr.h")
            message(STATUS "  Found system unarr at /usr/include")
            target_link_libraries(cbx_backend PRIVATE unarr)
        else()
            message(FATAL_ERROR
                "Could not find unarr. Install libunarr or use a different DECOMPRESSION_BACKEND.")
        endif()
    endif()

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

    # Try pkg-config first, then system path
    pkg_check_modules(LIBARCHIVE QUIET IMPORTED_TARGET libarchive)
    if(LIBARCHIVE_FOUND)
        message(STATUS "  Found libarchive via pkg-config")
        target_link_libraries(cbx_backend PRIVATE PkgConfig::LIBARCHIVE)
    elseif(EXISTS "/usr/include/archive.h")
        message(STATUS "  Found system libarchive at /usr/include")
        target_link_libraries(cbx_backend PRIVATE archive)
    else()
        message(FATAL_ERROR
            "Could not find libarchive. Install it or use a different DECOMPRESSION_BACKEND.")
    endif()

else()
    message(FATAL_ERROR
        "Unknown DECOMPRESSION_BACKEND: '${DECOMPRESSION_BACKEND}'. "
        "Use: unarr, 7zip, or libarchive")
endif()

# Qt, yr_global, and QsLog are needed by all backends
target_link_libraries(cbx_backend PRIVATE Qt::Core yr_global QsLog)
