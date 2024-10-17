add_executable(
  YACReaderLibraryServer
  main.cpp console_ui_library_creator.h console_ui_library_creator.cpp
  libraries_updater.h libraries_updater.cpp)

target_compile_definitions(
  YACReaderLibraryServer PRIVATE SERVER_RELEASE
                                 DATADIR="${CMAKE_INSTALL_FULL_DATADIR}")

target_include_directories(YACReaderLibraryServer PRIVATE ../YACReaderLibrary)

target_link_libraries(
  YACReaderLibraryServer
  PRIVATE comic_backend
          comic_vine
          common_all
          db_helper
          library_common
          QrCode
          QsLog
          Qt::Core5Compat
          Qt::Gui
          Qt::Network
          Qt::Sql
          server)
