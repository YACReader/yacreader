######################################################################
# Automatically generated by qmake (2.01a) dom 12. oct 20:47:48 2008
######################################################################

TEMPLATE = app
TARGET = YACReaderLibraryServer
CONFIG += console
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ../common \
                ./server \
                ./db

DEFINES += SERVER_RELEASE NOMINMAX YACREADER_LIBRARY QT_NO_DEBUG_OUTPUT

#load default build flags
#do a basic dependency check
include(headless_config.pri)

win32 {
    LIBS += -L../dependencies/poppler/lib -loleaut32 -lole32 -lshell32 -luser32
    LIBS += -lpoppler-qt5
    INCLUDEPATH += ../dependencies/poppler/include/qt5

    QMAKE_CXXFLAGS_RELEASE += /MP /Ob2 /Oi /Ot /GT /GL
    QMAKE_LFLAGS_RELEASE += /LTCG
    CONFIG -= embed_manifest_exe
}

unix:!macx{
INCLUDEPATH  += /usr/include/poppler/qt5
LIBS         += -L/usr/lib -lpoppler-qt5
}

macx{
LIBS += -framework Foundation -framework ApplicationServices -framework AppKit

OBJECTIVE_SOURCES += $$PWD/../common/pdf_comic.mm
HEADERS += $$PWD/../common/pdf_comic.h
CONFIG += objective_c
}

unix{
CONFIG += c++11
}

#CONFIG += release
CONFIG -= flat
QT += core sql network script

# Input
HEADERS += library_creator.h \
           package_manager.h \
           bundle_creator.h \
           db_helper.h \
           ./db/data_base_management.h \
           ../common/comic_db.h \
           ../common/folder.h \
           ../common/library_item.h \
           ../common/comic.h \
           ../common/bookmarks.h \
           ../common/qnaturalsorting.h \
           ../common/yacreader_global.h \
           yacreader_local_server.h \
           comics_remover.h \
           ../common/http_worker.h \
           yacreader_libraries.h \
           comic_files_manager.h \
           headless/console_ui_library_creator.h


SOURCES += library_creator.cpp \
           .\headless\main.cpp \
           package_manager.cpp \
           bundle_creator.cpp \
           db_helper.cpp \
           ./db/data_base_management.cpp \
           ../common/comic_db.cpp \
           ../common/folder.cpp \
           ../common/library_item.cpp \
           ../common/comic.cpp \
           ../common/bookmarks.cpp \
           ../common/qnaturalsorting.cpp \
           yacreader_local_server.cpp \
           comics_remover.cpp \
           ../common/http_worker.cpp \
           ../common/yacreader_global.cpp \
           yacreader_libraries.cpp \
           comic_files_manager.cpp \
           headless/console_ui_library_creator.cpp

				   
include(./server/server.pri)
CONFIG(7zip){
include(../compressed_archive/wrapper.pri)
} else:CONFIG(unarr) {
include(../compressed_archive/unarr/unarr-wrapper.pri)
} else {
	error(No compression backend specified. Did you mess with the build system?)
}
include(../QsLog/QsLog.pri)


TRANSLATIONS =  yacreaderlibraryserver_es.ts \
                yacreaderlibraryserver_ru.ts \
                yacreaderlibraryserver_pt.ts \
                yacreaderlibraryserver_fr.ts \
                yacreaderlibraryserver_nl.ts \
                yacreaderlibraryserver_tr.ts \
                yacreaderlibraryserver_de.ts \
                yacreaderlibraryserver_source.ts


RESOURCES += headless/images.qrc


Release:DESTDIR = ../release
Debug:DESTDIR = ../debug


unix:!macx {
#set install prefix if it's empty
isEmpty(PREFIX) {
	PREFIX = /usr
}

BINDIR = $$PREFIX/bin
LIBDIR = $$PREFIX/lib
DATADIR = $$PREFIX/share

DEFINES += "LIBDIR=\\\"$$LIBDIR\\\""  "DATADIR=\\\"$$DATADIR\\\"" "BINDIR=\\\"$$BINDIR\\\""

#MAKE INSTALL
INSTALLS += bin server translation #manpage

bin.path = $$BINDIR
isEmpty(DESTDIR) {
	bin.files = YACReaderLibraryServer
} else {
	bin.files = $$DESTDIR/YACReaderLibraryServer
}

server.path = $$DATADIR/yacreader
server.files = ../release/server

translation.path = $$DATADIR/yacreader/languages
translation.files = ../release/languages/yacreaderlibrary_*

#manpage.path = $$DATADIR/man/man1
#manpage.files = ../YACReaderLibrary.1
}
