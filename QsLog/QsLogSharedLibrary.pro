# This pro file will build QsLog as a shared library
include(QsLog.pri)

TARGET = QsLog
VERSION = "2.0.0"
QT -= gui
CONFIG -= console
CONFIG -= app_bundle
CONFIG += shared
TEMPLATE = lib

DESTDIR = $$PWD/build-QsLogShared
OBJECTS_DIR = $$DESTDIR/obj
MOC_DIR = $$DESTDIR/moc

win32 {
    DEFINES += QSLOG_IS_SHARED_LIBRARY
}

unix:!macx {
    # make install will install the shared object in the appropriate folders
    headers.files = QsLog.h QsLogDest.h QsLogLevel.h
    headers.path = /usr/include/$(QMAKE_TARGET)

    other_files.files = *.txt
    other_files.path = /usr/local/share/$(QMAKE_TARGET)

    contains(QT_ARCH, x86_64) {
        target.path = /usr/lib64
    } else {
        target.path = /usr/lib
    }

    INSTALLS += headers target other_files
}
