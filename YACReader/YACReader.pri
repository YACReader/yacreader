INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
INCLUDEPATH += .
INCLUDEPATH += $$PWD/../common \
                            $$PWD/../custom_widgets

CONFIG(legacy_gl_widget) {
    INCLUDEPATH += ../common/gl_legacy \
} else {
    INCLUDEPATH += ../common/gl \
}

win32 {
LIBS += -L$$PWD/../dependencies/poppler/lib -loleaut32 -lole32

LIBS += -lpoppler-qt5
INCLUDEPATH += ../dependencies/poppler/include/qt5

QMAKE_CXXFLAGS_RELEASE += /MP /Ob2 /Oi /Ot /GT
!CONFIG(no_opengl) {
	QMAKE_CXXFLAGS_RELEASE += /GL
}
QMAKE_LFLAGS_RELEASE += /LTCG
CONFIG -= embed_manifest_exe
}

unix:!macx{

INCLUDEPATH  += /usr/include/poppler/qt5
LIBS         += -L/usr/lib -lpoppler-qt5

!CONFIG(no_opengl) {
	LIBS += -lGLU
}

}

macx{
#INCLUDEPATH  += "/Volumes/Mac OS X Lion/usr/X11/include"
#isEqual(QT_MAJOR_VERSION, 5) {
#INCLUDEPATH  += /usr/local/include/poppler/qt5
#LIBS         += -L/usr/local/lib -lpoppler-qt5
#}
#else {
#INCLUDEPATH  += /usr/local/include/poppler/qt4
#LIBS         += -L/usr/local/lib -lpoppler-qt4
#}
CONFIG += objective_c
QT += macextras gui-private


LIBS += -framework Foundation -framework ApplicationServices -framework AppKit

OBJECTIVE_SOURCES += $$PWD/../common/pdf_comic.mm
HEADERS += $$PWD/../common/pdf_comic.h
}

QT += network widgets core
!CONFIG(no_opengl) {
	QT += opengl
}

#CONFIG += release
CONFIG -= flat

QT += multimedia

# Input
HEADERS += $$PWD/../common/comic.h \
    $$PWD/configuration.h \
    $$PWD/goto_dialog.h \
    $$PWD/magnifying_glass.h \
	$$PWD/main_window_viewer.h \
    $$PWD/viewer.h \
    $$PWD/goto_flow.h \
    $$PWD/options_dialog.h \
    $$PWD/../common/bookmarks.h \
    $$PWD/bookmarks_dialog.h \
    $$PWD/render.h \
    $$PWD/shortcuts_dialog.h \
	$$PWD/translator.h \
	$$PWD/goto_flow_widget.h \
	$$PWD/page_label_widget.h \
	$$PWD/goto_flow_toolbar.h \
	$$PWD/goto_flow_decorationbar.h \
	$$PWD/width_slider.h \
	$$PWD/notifications_label_widget.h \
	$$PWD/../common/pictureflow.h \
    $$PWD/../common/custom_widgets.h \
    $$PWD/../common/check_new_version.h \
	$$PWD/../common/qnaturalsorting.h \
	$$PWD/../common/yacreader_global.h \
	$$PWD/../common/onstart_flow_selection_dialog.h \
	$$PWD/../common/comic_db.h \
	$$PWD/../common/folder.h \
	$$PWD/../common/library_item.h \
	$$PWD/yacreader_local_client.h \
	$$PWD/../common/http_worker.h \
	$$PWD/../common/exit_check.h \
        $$PWD/../common/scroll_management.h

!CONFIG(no_opengl) {
    CONFIG(legacy_gl_widget) {
        message("using legacy YACReaderFlowGL (QGLWidget) header")
        HEADERS += ../common/gl_legacy/yacreader_flow_gl.h
    } else {
        HEADERS += ../common/gl/yacreader_flow_gl.h
    }
    HEADERS += 	$$PWD/goto_flow_gl.h
}

SOURCES += $$PWD/../common/comic.cpp \
    $$PWD/configuration.cpp \
    $$PWD/goto_dialog.cpp \
	$$PWD/magnifying_glass.cpp \
    $$PWD/main_window_viewer.cpp \
    $$PWD/viewer.cpp \
    $$PWD/goto_flow.cpp \
    $$PWD/options_dialog.cpp \
    $$PWD/../common/bookmarks.cpp \
    $$PWD/bookmarks_dialog.cpp \
    $$PWD/render.cpp \
    $$PWD/shortcuts_dialog.cpp \
	$$PWD/translator.cpp \
	$$PWD/goto_flow_widget.cpp \
	$$PWD/page_label_widget.cpp \
	$$PWD/goto_flow_toolbar.cpp \
	$$PWD/goto_flow_decorationbar.cpp \
	$$PWD/width_slider.cpp \
	$$PWD/notifications_label_widget.cpp \
    $$PWD/../common/pictureflow.cpp \
    $$PWD/../common/custom_widgets.cpp \
    $$PWD/../common/check_new_version.cpp \
	$$PWD/../common/qnaturalsorting.cpp \
	$$PWD/../common/onstart_flow_selection_dialog.cpp \
	$$PWD/../common/comic_db.cpp \
	$$PWD/../common/folder.cpp \
	$$PWD/../common/library_item.cpp \
	$$PWD/yacreader_local_client.cpp \
    $$PWD/../common/http_worker.cpp \
    $$PWD/../common/yacreader_global.cpp \
	$$PWD/../common/exit_check.cpp \
    $$PWD/../common/scroll_management.cpp

!CONFIG(no_opengl) {
    CONFIG(legacy_gl_widget) {
        message("using legacy YACReaderFlowGL (QGLWidget) source code")
        SOURCES += ../common/gl_legacy/yacreader_flow_gl.cpp
    } else {
        SOURCES += ../common/gl/yacreader_flow_gl.cpp
    }
    SOURCES += $$PWD/goto_flow_gl.cpp
}

include($$PWD/../custom_widgets/custom_widgets_yacreader.pri)
include($$PWD/../compressed_archive/wrapper.pri)
include($$PWD/../shortcuts_management/shortcuts_management.pri)

RESOURCES += $$PWD/yacreader_images.qrc \
    $$PWD/yacreader_files.qrc 
	
win32:RESOURCES += $$PWD/yacreader_images_win.qrc
unix:!macx:RESOURCES += $$PWD/yacreader_images_win.qrc
macx:RESOURCES += $$PWD/yacreader_images_osx.qrc
