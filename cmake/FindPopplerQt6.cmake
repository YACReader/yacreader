include(BackendHelpers)
include(FindPackageHandleStandardArgs)

set(PopplerQt6_PROVIDER "")
set(PopplerQt6_TARGET "")
set(PopplerQt6_INCLUDE_DIRS "")
set(PopplerQt6_LIBRARIES "")

if(NOT TARGET Poppler::Qt6)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(POPPLERQT6 QUIET IMPORTED_TARGET poppler-qt6)
        if(TARGET PkgConfig::POPPLERQT6)
            yacreader_add_imported_library(Poppler::Qt6
                TYPE INTERFACE
                LINK_LIBRARIES "PkgConfig::POPPLERQT6")
            set(PopplerQt6_PROVIDER "pkg-config")
            set(PopplerQt6_INCLUDE_DIRS "${POPPLERQT6_INCLUDE_DIRS}")
        endif()
    endif()
endif()

if(NOT TARGET Poppler::Qt6)
    find_path(POPPLERQT6_INCLUDE_DIR NAMES poppler-qt6.h PATH_SUFFIXES poppler/qt6)
    find_library(POPPLERQT6_LIBRARY NAMES poppler-qt6)
    if(POPPLERQT6_INCLUDE_DIR AND POPPLERQT6_LIBRARY)
        yacreader_add_imported_library(Poppler::Qt6
            TYPE UNKNOWN
            LOCATION "${POPPLERQT6_LIBRARY}"
            INCLUDE_DIR "${POPPLERQT6_INCLUDE_DIR}")
        set(PopplerQt6_PROVIDER "CMake search")
        set(PopplerQt6_INCLUDE_DIRS "${POPPLERQT6_INCLUDE_DIR}")
    endif()
endif()

if(TARGET Poppler::Qt6)
    set(PopplerQt6_TARGET Poppler::Qt6)
    set(PopplerQt6_LIBRARIES Poppler::Qt6)
endif()

find_package_handle_standard_args(PopplerQt6 REQUIRED_VARS PopplerQt6_TARGET)

mark_as_advanced(POPPLERQT6_INCLUDE_DIR POPPLERQT6_LIBRARY)
