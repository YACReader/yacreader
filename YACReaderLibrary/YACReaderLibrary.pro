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

    msvc {
        QMAKE_CXXFLAGS_RELEASE += /MP /Ob2 /Oi /Ot /GT /GL
        QMAKE_LFLAGS_RELEASE += /LTCG
    }
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
  QT += gui-private
  lessThan(QT_MAJOR_VERSION, 6): QT += macextras
}

#CONFIG += release
CONFIG -= flat
QT += sql network widgets svg quickcontrols2

greaterThan(QT_MAJOR_VERSION, 5): QT += openglwidgets core5compat

# Input
HEADERS += comic_flow.h \
  ../common/concurrent_queue.h \
  create_library_dialog.h \
  db/comic_query_result_processor.h \
  db/folder_query_result_processor.h \
  db/query_lexer.h \
  db/search_query.h \
  folder_content_view.h \
  initial_comic_info_extractor.h \
  libraries_update_coordinator.h \
  library_comic_opener.h \
  library_creator.h \
  library_window.h \
  add_library_dialog.h \
  library_window_actions.h \
  recent_visibility_coordinator.h \
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
  ../common/pdf_comic.h \
  no_libraries_widget.h \
  import_widget.h \
  trayicon_controller.h \
  xml_info_library_scanner.h \
  xml_info_parser.h \
  yacreader_content_views_manager.h \
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
  info_comics_view.h \
  yacreader_comics_selection_helper.h \
  yacreader_comic_info_helper.h \
  db/reading_list.h \
  db/query_parser.h \
  current_comic_view_helper.h \
  ip_config_helper.h \
  ../common/global_info_provider.h \

!CONFIG(no_opengl) {
        HEADERS += ../common/gl/yacreader_flow_gl.h
}

SOURCES += comic_flow.cpp \
    ../common/concurrent_queue.cpp \
    create_library_dialog.cpp \
    db/comic_query_result_processor.cpp \
    db/folder_query_result_processor.cpp \
    db/query_lexer.cpp \
    db/search_query.cpp \
    folder_content_view.cpp \
    initial_comic_info_extractor.cpp \
    libraries_update_coordinator.cpp \
    library_comic_opener.cpp \
    library_creator.cpp \
    library_window.cpp \
    library_window_actions.cpp \
    main.cpp \
    add_library_dialog.cpp \
    recent_visibility_coordinator.cpp \
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
    no_libraries_widget.cpp \
    import_widget.cpp \
    trayicon_controller.cpp \
    xml_info_library_scanner.cpp \
    xml_info_parser.cpp \
    yacreader_content_views_manager.cpp \
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
    info_comics_view.cpp \
    yacreader_comics_selection_helper.cpp \
    yacreader_comic_info_helper.cpp\
    db/reading_list.cpp \
    current_comic_view_helper.cpp \
    db/query_parser.cpp \
    ip_config_helper.cpp \
    ../common/global_info_provider.cpp \

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
} else:CONFIG(libarchive) {
include(../compressed_archive/libarchive/libarchive-wrapper.pri)
} else {
  error(No compression backend specified. Did you mess with the build system?)
}

include(./comic_vine/comic_vine.pri)
include(../third_party/QsLog/QsLog.pri)
include(../shortcuts_management/shortcuts_management.pri)
include(../third_party/QrCode/QrCode.pri)
include(../third_party/KDToolBox/KDToolBox.pri)

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
                yacreaderlibrary_zh_CN.ts \
                yacreaderlibrary_zh_TW.ts \
                yacreaderlibrary_zh_HK.ts \
                yacreaderlibrary_it.ts \
                yacreaderlibrary_en.ts

CONFIG += lrelease

win32 {
    CONFIG(release, debug|release) {
        SOURCE_QM_DIR = $$OUT_PWD/release/*.qm
    }
    CONFIG(debug, debug|release) {
        SOURCE_QM_DIR = $$OUT_PWD/debug/*.qm
    }

    DEPLOYMENT_OUT_QM_DIR = ../release/languages/
    OUT_QM_DIR = $${DESTDIR}/languages/

    QMAKE_POST_LINK += $(MKDIR) $$shell_path($${OUT_QM_DIR}) 2> NULL & \
                       $(COPY) $$shell_path($${SOURCE_QM_DIR}) $$shell_path($${OUT_QM_DIR}) & \
                       $(MKDIR) $$shell_path($${DEPLOYMENT_OUT_QM_DIR}) 2> NULL & \
                       $(COPY) $$shell_path($${SOURCE_QM_DIR}) $$shell_path($${DEPLOYMENT_OUT_QM_DIR})
} else {
    LRELEASE_DIR = ../release/languages/
    QM_FILES_INSTALL_PATH = $$DATADIR/yacreader/languages
}

#QML/GridView
QT += quick qml quickwidgets

HEADERS += grid_comics_view.h \
           comics_view_transition.h

SOURCES += grid_comics_view.cpp \
           comics_view_transition.cpp

greaterThan(QT_MAJOR_VERSION, 5) {
    RESOURCES += qml6.qrc
} else {
    RESOURCES += qml.qrc
}
win32:RESOURCES += qml_win.qrc
unix:!macx:RESOURCES += qml_win.qrc
macx:RESOURCES += qml_osx.qrc

unix:!macx {

DEFINES += "LIBDIR=\\\"$$LIBDIR\\\""  "DATADIR=\\\"$$DATADIR\\\"" "BINDIR=\\\"$$BINDIR\\\""

#MAKE INSTALL
INSTALLS += bin icon desktop server manpage

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

manpage.path = $$DATADIR/man/man1
manpage.files = ../YACReaderLibrary.1
}
