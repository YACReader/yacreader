TEMPLATE = app
TARGET = YACReaderLibrary

QMAKE_TARGET_BUNDLE_PREFIX = "com.yacreader"

DEPENDPATH += .
INCLUDEPATH += . \
              ../common \
              ./server \
              ./db \
              ../custom_widgets \
              ./comic_vine \
              ./comic_vine/model

DEFINES += SERVER_RELEASE YACREADER_LIBRARY

# load default build flags
include (../config.pri)
include (../dependencies/pdf_backend.pri)

unix:haiku {
  DEFINES += _BSD_SOURCE
  LIBS    += -lnetwork -lbsd
}

INCLUDEPATH += ../common/gl

# there are two builds for Windows, Desktop OpenGL based and ANGLE OpenGL ES based
win32 {
    CONFIG(force_angle) {
        message("using ANGLE")
        LIBS += -loleaut32 -lole32 -lshell32 -lopengl32 -luser32
        # linking extra libs are necesary for a successful compilation, a better approach should be
        # to remove any OpenGL (desktop) dependencies
        # the OpenGL stuff should be migrated to OpenGL ES
        DEFINES += FORCE_ANGLE
    } else {
        LIBS += -loleaut32 -lole32 -lshell32 -lopengl32 -luser32
    }

    QMAKE_CXXFLAGS_RELEASE += /MP /Ob2 /Oi /Ot /GT /GL
    QMAKE_LFLAGS_RELEASE += /LTCG
    CONFIG -= embed_manifest_exe
}

CONFIG(force_angle) {
    contains(QMAKE_TARGET.arch, x86_64) {
      Release:DESTDIR = ../release64_angle
      Debug:DESTDIR = ../debug64_angle
    } else {
      Release:DESTDIR = ../release_angle
      Debug:DESTDIR = ../debug_angle
    }
} else {
    contains(QMAKE_TARGET.arch, x86_64) {
      Release:DESTDIR = ../release64
      Debug:DESTDIR = ../debug64
    } else {
      Release:DESTDIR = ../release
      Debug:DESTDIR = ../debug
    }
}

macx {
  LIBS += -framework Foundation -framework ApplicationServices -framework AppKit
  CONFIG += objective_c
  QT += macextras gui-private
}

#CONFIG += release
CONFIG -= flat
QT += sql network widgets svg

# Input
HEADERS += comic_flow.h \
  ../common/concurrent_queue.h \
  create_library_dialog.h \
  db/comic_query_result_processor.h \
  db/folder_query_result_processor.h \
  db/query_lexer.h \
  library_creator.h \
  library_window.h \
  add_library_dialog.h \
  rename_library_dialog.h \
  properties_dialog.h \
  options_dialog.h \
  export_library_dialog.h \
  import_library_dialog.h \
  package_manager.h \
  bundle_creator.h \
  export_comics_info_dialog.h \
  import_comics_info_dialog.h \
  server_config_dialog.h \
  comic_flow_widget.h \
  db_helper.h \
  ./db/data_base_management.h \
  ./db/folder_item.h \
  ./db/folder_model.h \
  ./db/comic_model.h \
  ./db/comic_item.h \
  ../common/comic_db.h \
  ../common/folder.h \
  ../common/library_item.h \
  ../common/comic.h \
  ../common/bookmarks.h \
  ../common/pictureflow.h \
  ../common/release_acquire_atomic.h \
  ../common/worker_thread.h \
  ../common/custom_widgets.h \
  ../common/qnaturalsorting.h \
  ../common/yacreader_global.h \
  ../common/yacreader_global_gui.h \
  ../common/onstart_flow_selection_dialog.h \
  ../common/pdf_comic.h \
  no_libraries_widget.h \
  import_widget.h \
  trayicon_controller.h \
  yacreader_local_server.h \
  yacreader_main_toolbar.h \
  comics_remover.h \
  ../common/http_worker.h \
  yacreader_libraries.h \
  ../common/exit_check.h \
  comics_view.h \
  classic_comics_view.h \
  empty_folder_widget.h \
  no_search_results_widget.h \
  comic_files_manager.h \
  db/reading_list_model.h \
  db/reading_list_item.h \
  yacreader_folders_view.h \
  yacreader_reading_lists_view.h \
  add_label_dialog.h \
  yacreader_history_controller.h \
  yacreader_navigation_controller.h \
  empty_label_widget.h \
  empty_container_info.h \
  empty_special_list.h \
  empty_reading_list_widget.h \
  ../common/scroll_management.h \
  ../common/opengl_checker.h \
  yacreader_comics_views_manager.h \
  info_comics_view.h \
  yacreader_comics_selection_helper.h \
  yacreader_comic_info_helper.h \
  db/reading_list.h \
  db/query_parser.h \
  current_comic_view_helper.h

!CONFIG(no_opengl) {
        HEADERS += ../common/gl/yacreader_flow_gl.h
}

SOURCES += comic_flow.cpp \
    ../common/concurrent_queue.cpp \
    create_library_dialog.cpp \
    db/comic_query_result_processor.cpp \
    db/folder_query_result_processor.cpp \
    db/query_lexer.cpp \
    library_creator.cpp \
    library_window.cpp \
    main.cpp \
    add_library_dialog.cpp \
    rename_library_dialog.cpp \
    properties_dialog.cpp \
    options_dialog.cpp \
    export_library_dialog.cpp \
    import_library_dialog.cpp \
    package_manager.cpp \
    bundle_creator.cpp \
    export_comics_info_dialog.cpp \
    import_comics_info_dialog.cpp \
    server_config_dialog.cpp \
    comic_flow_widget.cpp \
    db_helper.cpp \
    ./db/data_base_management.cpp \
    ./db/folder_item.cpp \
    ./db/folder_model.cpp \
    ./db/comic_model.cpp \
    ./db/comic_item.cpp \
    ../common/comic_db.cpp \
    ../common/folder.cpp \
    ../common/library_item.cpp \
    ../common/comic.cpp \
    ../common/bookmarks.cpp \
    ../common/pictureflow.cpp \
    ../common/custom_widgets.cpp \
    ../common/qnaturalsorting.cpp \
    ../common/onstart_flow_selection_dialog.cpp \
    no_libraries_widget.cpp \
    import_widget.cpp \
    trayicon_controller.cpp \
    yacreader_local_server.cpp \
    yacreader_main_toolbar.cpp \
    comics_remover.cpp \
    ../common/http_worker.cpp \
    ../common/yacreader_global.cpp \
    ../common/yacreader_global_gui.cpp \
    yacreader_libraries.cpp \
    ../common/exit_check.cpp \
    comics_view.cpp \
    classic_comics_view.cpp \
    empty_folder_widget.cpp \
    no_search_results_widget.cpp \
    comic_files_manager.cpp \
    db/reading_list_model.cpp \
    db/reading_list_item.cpp \
    yacreader_folders_view.cpp \
    yacreader_reading_lists_view.cpp \
    add_label_dialog.cpp \
    yacreader_history_controller.cpp \
    yacreader_navigation_controller.cpp \
    empty_label_widget.cpp \
    empty_container_info.cpp \
    empty_special_list.cpp \
    empty_reading_list_widget.cpp \
    ../common/scroll_management.cpp \
    ../common/opengl_checker.cpp \
    yacreader_comics_views_manager.cpp \
    info_comics_view.cpp \
    yacreader_comics_selection_helper.cpp \
    yacreader_comic_info_helper.cpp\
    db/reading_list.cpp \
    current_comic_view_helper.cpp \
    db/query_parser.cpp

!CONFIG(no_opengl) {
    SOURCES += ../common/gl/yacreader_flow_gl.cpp
}

macx {
   HEADERS += trayhandler.h
   OBJECTIVE_SOURCES += trayhandler.mm
}

include(./server/server.pri)
include(../custom_widgets/custom_widgets_yacreaderlibrary.pri)

CONFIG(7zip){
include(../compressed_archive/wrapper.pri)
} else:CONFIG(unarr) {
include(../compressed_archive/unarr/unarr-wrapper.pri)
} else {
  error(No compression backend specified. Did you mess with the build system?)
}

include(./comic_vine/comic_vine.pri)
include(../third_party/QsLog/QsLog.pri)
include(../shortcuts_management/shortcuts_management.pri)

RESOURCES += images.qrc files.qrc
win32:RESOURCES += images_win.qrc
unix:!macx:RESOURCES += images_win.qrc
macx:RESOURCES += images_osx.qrc

RC_FILE = icon.rc

macx {
  ICON = YACReaderLibrary.icns
  QMAKE_INFO_PLIST = Info.plist
}

TRANSLATIONS =   yacreaderlibrary_es.ts \
                yacreaderlibrary_ru.ts \
                yacreaderlibrary_pt.ts \
                yacreaderlibrary_fr.ts \
                yacreaderlibrary_nl.ts \
                yacreaderlibrary_tr.ts \
                yacreaderlibrary_de.ts \
                yacreaderlibrary_zh.ts \
                yacreaderlibrary_it.ts \
                yacreaderlibrary_source.ts

#QML/GridView
QT += quick qml quickwidgets

HEADERS += grid_comics_view.h \
           comics_view_transition.h

SOURCES += grid_comics_view.cpp \
           comics_view_transition.cpp

RESOURCES += qml.qrc
win32:RESOURCES += qml_win.qrc
unix:!macx:RESOURCES += qml_win.qrc
macx:RESOURCES += qml_osx.qrc

unix:!macx {
#set install prefix if it's empty
isEmpty(PREFIX) {
  PREFIX = /usr
}
isEmpty(BINDIR) {
  BINDIR = $$PREFIX/bin
}
isEmpty(LIBDIR) {
  LIBDIR = $$PREFIX/lib
}
isEmpty(DATADIR) {
  DATADIR = $$PREFIX/share
}

DEFINES += "LIBDIR=\\\"$$LIBDIR\\\""  "DATADIR=\\\"$$DATADIR\\\""

DEFINES += "LIBDIR=\\\"$$LIBDIR\\\""  "DATADIR=\\\"$$DATADIR\\\"" "BINDIR=\\\"$$BINDIR\\\""

#MAKE INSTALL
INSTALLS += bin icon desktop server translation manpage

bin.path = $$BINDIR
isEmpty(DESTDIR) {
  bin.files = YACReaderLibrary
} else {
  bin.files = $$DESTDIR/YACReaderLibrary
}

server.path = $$DATADIR/yacreader
server.files = ../release/server

icon.path = $$DATADIR/icons/hicolor/scalable/apps
icon.files = ../YACReaderLibrary.svg

desktop.path = $$DATADIR/applications
desktop.files = ../YACReaderLibrary.desktop

translation.path = $$DATADIR/yacreader/languages
translation.files = ../release/languages/yacreaderlibrary_*

manpage.path = $$DATADIR/man/man1
manpage.files = ../YACReaderLibrary.1
}
