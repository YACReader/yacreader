cmake_minimum_required(VERSION 3.16)

project(
  YACReader
  VERSION 9.13.1
  LANGUAGES C CXX)

# enable Objective-C for macOSX
if(APPLE)
  enable_language(OBJC)
  enable_language(OBJCXX)
endif()

# Extra functions
find_package(PkgConfig)
include(GNUInstallDirs)

# Set defaults TODO: Bad practice?
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
  add_compile_options("-we4061")
else()
  add_compile_options("-Werror=switch")
endif()

# Set default build type to "Release" if it is empty
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(
    STATUS
      "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE
      "${default_build_type}"
      CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
                                               "MinSizeRel" "RelWithDebInfo")
endif()

# Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
  message(
    FATAL_ERROR
      "You cannot build in a source directory (or any \
        directory with a CMakeLists.txt file). Please make a build subdirectory. \
        Feel free to remove CMakeCache.txt and CMakeFiles.
        ")
endif()

# Common build Options
if(NOT DECOMPRESSION_BACKEND)
  set(DECOMPRESSION_BACKEND
      "unarr"
      CACHE STRING "Decompression backend for cbx files")
endif()
set_property(CACHE DECOMPRESSION_BACKEND PROPERTY STRINGS "unarr" "7zip"
                                                  "libarchive")

if(NOT PDF_BACKEND)
  set(PDF_BACKEND
      "pdfium"
      CACHE STRING "PDF comic rendering backend")
endif()
set_property(CACHE PDF_BACKEND PROPERTY STRINGS "pdfium" "poppler" "pdfkit")

# TODO: OpenGL
if(NOT OPENGL)
  set(OPENGL
      "desktop"
      CACHE STRING "OpenGL backend")
endif()
set_property(CACHE OPENGL PROPERTY STRINGS "desktop" "angle" "force-angle"
                                   "no_opengl")

# TODO: option(SERVER_STANDALONE "Server standalone build (no gui apps)" OFF)

# Handle dependencies for backends
if(DECOMPRESSION_BACKEND STREQUAL "unarr")
  find_package("unarr" REQUIRED)
elseif(DECOMPRESSION_BACKEND STREQUAL "7zip")
  find_package("7zip" REQUIRED)
elseif(DECOMPRESSION_BACKEND STREQUAL "libarchive")
  find_package("LibArchive::LibArchive" REQUIRED)
else()
  message(FATAL_ERROR "No valid decompression backend specified.")
endif()

if(PDF_BACKEND STREQUAL "pdfium")
  pkg_check_modules(PDFIUM IMPORTED_TARGET "libpdfium" REQUIRED)
elseif(PDF_BACKEND STREQUAL "poppler")
  pkg_check_modules(POPPLER IMPORTED_TARGET "poppler-qt6" REQUIRED)
elseif(PDF_BACKEND STREQUAL "pdfkit")
  find_package("pdfkit" REQUIRED)
elseif(PDF_BACKEND STREQUAL "no_pdf")
  # Do nothing
else()
  message(FATAL_ERROR "No valid PDF backend specified.")
endif()

# Qt6 modules TODO: Server standalone
find_package(
  Qt6 REQUIRED
  COMPONENTS Core
             Core5Compat
             Gui
             Network
             LinguistTools
             Multimedia
             OpenGLWidgets
             QuickControls2
             QuickWidgets
             Svg
             Sql
             Widgets)

# Qt project setup

# TODO: It might be better to do this manually
qt_standard_project_setup()

# Modules
add_subdirectory(common)
add_subdirectory(compressed_archive)
add_subdirectory(custom_widgets)
add_subdirectory(shortcuts_management)
add_subdirectory(third_party)
add_subdirectory(YACReaderLibrary/server)

# Executables
add_subdirectory(YACReader)
add_subdirectory(YACReaderLibrary)
add_subdirectory(YACReaderLibraryServer)

# Install and Deployment
if(UNIX AND NOT APPLE)
  if(SERVER_STANDALONE)
    install(
      TARGETS YACReaderLibraryServer
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/yacreader)
  else()
    install(
      TARGETS YACReader YACReaderLibrary YACReaderLibraryServer
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/yacreader)
  endif()
  # Docs and runtime
  install(DIRECTORY release DESTINATION ${CMAKE_INSTALL_DATADIR}/yacreader)
  install(FILES README.md CHANGELOG.md DESTINATION ${CMAKE_INSTALL_DOCDIR})
  # Manpages
  install(FILES YACReader.1 YACReaderLibrary.1
          DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
  # Icons, Desktop files
  install(FILES YACReader.desktop YACReaderLibrary.desktop
          DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
  install(FILES YACReader.svg YACReaderLibrary.svg
          DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
  # Translations
  install(FILES ${reader_translations} ${library_translations}
          DESTINATION ${CMAKE_INSTALL_DATADIR}/yacreader/release/languages)
elseif(WIN32)
  # TODO
elseif(APPLE)
  # TODO
endif()
