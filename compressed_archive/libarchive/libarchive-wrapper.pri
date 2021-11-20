INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += $$PWD/extract_delegate.h \
           $$PWD/compressed_archive.h

SOURCES += $$PWD/compressed_archive.cpp

if(mingw|unix):!contains(QT_CONFIG, no-pkg-config):packagesExist(libarchive) {
  message(Using system provided libarchive installation found by pkg-config.)
  CONFIG += link_pkgconfig
  PKGCONFIG += libarchive
  DEFINES += use_libarchive
  }
else:unix:exists(/usr/include/archive.h) {
  message(Using system provided libarchive installation.)
  LIBS += -larchive
  DEFINES += use_libarchive
}
else {
  error(Missing dependency: libarchive decompression backend. Please install libarchive on your system)
}
