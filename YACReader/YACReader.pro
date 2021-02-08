TEMPLATE = app
TARGET = YACReader

QMAKE_TARGET_BUNDLE_PREFIX = "com.yacreader"

DEPENDPATH += . \
    release

DEFINES += NOMINMAX YACREADER

#load default build flags
include (../config.pri)
include (../dependencies/pdf_backend.pri)

unix:haiku {
  DEFINES += _BSD_SOURCE
  LIBS    += -lnetwork -lbsd
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

SOURCES += main.cpp

INCLUDEPATH += ../common \
               ../custom_widgets

!CONFIG(no_opengl) {
    INCLUDEPATH += ../common/gl
}

#there are going to be two builds for windows, OpenGL based and ANGLE based
win32 {
    CONFIG(force_angle) {
        message("using ANGLE")
        LIBS += -loleaut32 -lole32 -lshell32 -lopengl32 -luser32
        #linking extra libs are necesary for a successful compilation, a better approach should be
        #to remove any OpenGL (desktop) dependencies
        #the OpenGL stuff should be migrated to OpenGL ES
        DEFINES += FORCE_ANGLE
    } else {
        LIBS += -loleaut32 -lole32 -lshell32 -lopengl32 -luser32
    }

    QMAKE_CXXFLAGS_RELEASE += /MP /Ob2 /Oi /Ot /GT /GL
    QMAKE_LFLAGS_RELEASE += /LTCG
    CONFIG -= embed_manifest_exe
}

macx {
    QT += macextras gui-private
    CONFIG += objective_c
    LIBS += -framework Foundation -framework ApplicationServices -framework AppKit
}

QT += network widgets core multimedia svg
!CONFIG(no_opengl) {
    QT += opengl
}

#CONFIG += release
CONFIG -= flat

# Sources
HEADERS +=  ../common/comic.h \
            configuration.h \
            goto_dialog.h \
            magnifying_glass.h \
            main_window_viewer.h \
            viewer.h \
            goto_flow.h \
            options_dialog.h \
            ../common/bookmarks.h \
            bookmarks_dialog.h \
            render.h \
            shortcuts_dialog.h \
            translator.h \
            goto_flow_widget.h \
            page_label_widget.h \
            goto_flow_toolbar.h \
            width_slider.h \
            notifications_label_widget.h \
            ../common/pictureflow.h \
            ../common/custom_widgets.h \
            ../common/check_new_version.h \
            ../common/qnaturalsorting.h \
            ../common/yacreader_global.h \
            ../common/yacreader_global_gui.h \
            ../common/onstart_flow_selection_dialog.h \
            ../common/comic_db.h \
            ../common/folder.h \
            ../common/library_item.h \
            yacreader_local_client.h \
            ../common/http_worker.h \
            ../common/exit_check.h \
            ../common/scroll_management.h \
            ../common/opengl_checker.h \
            ../common/pdf_comic.h

!CONFIG(no_opengl) {
    HEADERS += ../common/gl/yacreader_flow_gl.h \
                goto_flow_gl.h
}

SOURCES +=  ../common/comic.cpp \
            configuration.cpp \
            goto_dialog.cpp \
            magnifying_glass.cpp \
            main_window_viewer.cpp \
            viewer.cpp \
            goto_flow.cpp \
            options_dialog.cpp \
            ../common/bookmarks.cpp \
            bookmarks_dialog.cpp \
            render.cpp \
            shortcuts_dialog.cpp \
            translator.cpp \
            goto_flow_widget.cpp \
            page_label_widget.cpp \
            goto_flow_toolbar.cpp \
            width_slider.cpp \
            notifications_label_widget.cpp \
            ../common/pictureflow.cpp \
            ../common/custom_widgets.cpp \
            ../common/check_new_version.cpp \
            ../common/qnaturalsorting.cpp \
            ../common/onstart_flow_selection_dialog.cpp \
            ../common/comic_db.cpp \
            ../common/folder.cpp \
            ../common/library_item.cpp \
            yacreader_local_client.cpp \
            ../common/http_worker.cpp \
            ../common/yacreader_global.cpp \
            ../common/yacreader_global_gui.cpp \
            ../common/exit_check.cpp \
            ../common/scroll_management.cpp \
            ../common/opengl_checker.cpp

!CONFIG(no_opengl) {
        SOURCES += ../common/gl/yacreader_flow_gl.cpp \
                    goto_flow_gl.cpp
}

include(../custom_widgets/custom_widgets_yacreader.pri)
CONFIG(7zip){
include(../compressed_archive/wrapper.pri)
} else:CONFIG(unarr){
include(../compressed_archive/unarr/unarr-wrapper.pri)
} else {
  error(No compression backend specified. Did you mess with the build system?)
  }
include(../shortcuts_management/shortcuts_management.pri)

RESOURCES += yacreader_images.qrc \
    yacreader_files.qrc

win32:RESOURCES += yacreader_images_win.qrc
unix:!macx:RESOURCES += yacreader_images_win.qrc
macx:RESOURCES += yacreader_images_osx.qrc

include(../third_party/QsLog/QsLog.pri)

RC_FILE = icon.rc

macx {
  ICON = YACReader.icns
  QMAKE_INFO_PLIST = Info.plist
}

TRANSLATIONS =    yacreader_es.ts \
                  yacreader_fr.ts \
                  yacreader_ru.ts \
                  yacreader_pt.ts \
                  yacreader_nl.ts \
                  yacreader_tr.ts \
                  yacreader_de.ts \
                  yacreader_source.ts

unix:!macx {
# set install prefix if it's empty
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

#MAKE INSTALL

INSTALLS += bin docs icon desktop translation manpage

bin.path = $$BINDIR
isEmpty(DESTDIR) {
  bin.files = YACReader
} else {
  bin.files = $$DESTDIR/YACReader
}

docs.path = $$DATADIR/doc/yacreader

#rename docs for better packageability
docs.extra = cp ../README.md ../README
docs.files = ../README ../CHANGELOG.md

icon.path = $$DATADIR/icons/hicolor/scalable/apps
icon.files = ../YACReader.svg

desktop.path = $$DATADIR/applications
desktop.files = ../YACReader.desktop

translation.path = $$DATADIR/yacreader/languages
translation.files = ../release/languages/yacreader_*

manpage.path = $$DATADIR/man/man1
manpage.files = ../YACReader.1

#remove leftover doc files when 'make clean' is invoked
QMAKE_CLEAN += "../README"
}
