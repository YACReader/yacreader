TEMPLATE = app
TARGET = YACReaderLibraryServer

QMAKE_TARGET_BUNDLE_PREFIX = "com.yacreader"

CONFIG += console
DEPENDPATH += ../YACReaderLibrary
INCLUDEPATH += ../YACReaderLibrary \
                ../common \
                ../YACReaderLibrary/server \
                ../YACReaderLibrary/db

DEFINES += SERVER_RELEASE NOMINMAX YACREADER_LIBRARY
# load default build flags
# do a basic dependency check
include(headless_config.pri)
include(../dependencies/pdf_backend.pri)

greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

win32 {
  LIBS += -loleaut32 -lole32 -lshell32 -luser32
  QMAKE_CXXFLAGS_RELEASE += /MP /Ob2 /Oi /Ot /GT /GL
  QMAKE_LFLAGS_RELEASE += /LTCG
  CONFIG -= embed_manifest_exe
}

macx {
  LIBS += -framework Foundation -framework ApplicationServices -framework AppKit
  CONFIG += objective_c
}

unix:haiku {
  DEFINES += _BSD_SOURCE
  LIBS    += -lnetwork -lbsd
}

#CONFIG += release
CONFIG -= flat
QT += core sql network

greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

# Source files
HEADERS += ../YACReaderLibrary/library_creator.h \
           ../YACReaderLibrary/package_manager.h \
           ../YACReaderLibrary/bundle_creator.h \
           ../YACReaderLibrary/db_helper.h \
           ../YACReaderLibrary/db/data_base_management.h \
           ../YACReaderLibrary/db/reading_list.h \
           ../YACReaderLibrary/initial_comic_info_extractor.h \
           ../YACReaderLibrary/xml_info_parser.h \
           ../common/comic_db.h \
           ../common/folder.h \
           ../common/library_item.h \
           ../common/comic.h \
           ../common/pdf_comic.h \
           ../common/bookmarks.h \
           ../common/qnaturalsorting.h \
           ../common/yacreader_global.h \
           ../YACReaderLibrary/yacreader_local_server.h \
           ../YACReaderLibrary/comics_remover.h \
           ../common/http_worker.h \
           ../YACReaderLibrary/yacreader_libraries.h \
           ../YACReaderLibrary/comic_files_manager.h \
           console_ui_library_creator.h \
           libraries_updater.h


SOURCES += ../YACReaderLibrary/library_creator.cpp \
           ../YACReaderLibrary/package_manager.cpp \
           ../YACReaderLibrary/bundle_creator.cpp \
           ../YACReaderLibrary/db_helper.cpp \
           ../YACReaderLibrary/db/data_base_management.cpp \
           ../YACReaderLibrary/db/reading_list.cpp \
           ../YACReaderLibrary/initial_comic_info_extractor.cpp \
           ../YACReaderLibrary/xml_info_parser.cpp \
           ../common/comic_db.cpp \
           ../common/folder.cpp \
           ../common/library_item.cpp \
           ../common/comic.cpp \
           ../common/bookmarks.cpp \
           ../common/qnaturalsorting.cpp \
           ../YACReaderLibrary/yacreader_local_server.cpp \
           ../YACReaderLibrary/comics_remover.cpp \
           ../common/http_worker.cpp \
           ../common/yacreader_global.cpp \
           ../YACReaderLibrary/yacreader_libraries.cpp \
           ../YACReaderLibrary/comic_files_manager.cpp \
           console_ui_library_creator.cpp \
           main.cpp \
           libraries_updater.cpp

include(../YACReaderLibrary/server/server.pri)
CONFIG(7zip) {
include(../compressed_archive/wrapper.pri)
} else:CONFIG(unarr) {
include(../compressed_archive/unarr/unarr-wrapper.pri)
} else {
  error(No compression backend specified. Did you mess with the build system?)
}
include(../third_party/QsLog/QsLog.pri)

TRANSLATIONS =  yacreaderlibraryserver_es.ts \
                yacreaderlibraryserver_ru.ts \
                yacreaderlibraryserver_pt.ts \
                yacreaderlibraryserver_fr.ts \
                yacreaderlibraryserver_nl.ts \
                yacreaderlibraryserver_tr.ts \
                yacreaderlibraryserver_de.ts \
                yacreaderlibraryserver_source.ts

RESOURCES += images.qrc

contains(QMAKE_TARGET.arch, x86_64) {
  Release:DESTDIR = ../release64
  Debug:DESTDIR = ../debug64
} else {
  Release:DESTDIR = ../release
  Debug:DESTDIR = ../debug
}

unix:!macx {
#set install prefix if it's empty
isEmpty(PREFIX) {
  PREFIX = /usr
}

BINDIR = $$PREFIX/bin
LIBDIR = $$PREFIX/lib
DATADIR = $$PREFIX/share

DEFINES += "LIBDIR=\\\"$$LIBDIR\\\""  "DATADIR=\\\"$$DATADIR\\\"" "BINDIR=\\\"$$BINDIR\\\""

#make install

!CONFIG(server_bundled):!CONFIG(server_standalone): {
    CONFIG+=server_bundled
    message("No build type specified. Defaulting to bundled server build (CONFIG+=server_bundled).")
    message("If you wish to run YACReaderLibraryServer on a system without an existing install of YACReaderLibrary,\
            please specify CONFIG+=server_standalone as an option when running qmake to avoid missing dependencies.")
}

CONFIG(server_standalone) {
  INSTALLS += bin server translation systemd
}
else:CONFIG(server_bundled) {
  INSTALLS += bin systemd
}

bin.path = $$BINDIR
isEmpty(DESTDIR) {
  bin.files = YACReaderLibraryServer
} else {
  bin.files = $$DESTDIR/YACReaderLibraryServer
}

server.path = $$DATADIR/yacreader
server.files = ../release/server

systemd.path = $$LIBDIR/systemd/user
systemd.files = yacreaderlibraryserver.service

translation.path = $$DATADIR/yacreader/languages
translation.files = ../release/languages/yacreaderlibrary_*

# TODO: We need a manpage for yaclibserver
#manpage.path = $$DATADIR/man/man1
#manpage.files = ../YACReaderLibrary.1
}
