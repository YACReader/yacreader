INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += $$PWD/extract_delegate.h \
           $$PWD/compressed_archive.h

SOURCES += $$PWD/compressed_archive.cpp

if(mingw|unix):!macx:!contains(QT_CONFIG, no-pkg-config):packagesExist(libarchive) {
  message(Using system provided libarchive installation found by pkg-config.)
  !system(pkg-config --atleast-version=3.6.0 libarchive) {
    LIBARCHIVE_WARNING = "libarchive < 3.6.0 found. Older versions of libarchive DO NOT SUPPORT RARv4 files. This is probably not what you want"
    warning($$LIBARCHIVE_WARNING)
    message($$LIBARCHIVE_WARNING)
  }

  CONFIG += link_pkgconfig
  PKGCONFIG += libarchive
  DEFINES += use_libarchive
  }
else:unix:!macx:exists(/usr/include/archive.h) {
  message(Using system provided libarchive installation.)
  LIBS += -larchive
  DEFINES += use_libarchive
}
else:if(win32|macx) {
  error(Unsupported: the libarchive decompression backend is not currently supported on this system.)
}
else {
  error(Missing dependency: libarchive decompression backend. Please install libarchive on your system)
}
