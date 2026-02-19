# functions to automatically initialize some of YACReader's build options to
# default values if they're not set on build time
# for a more detailed description, see INSTALL.TXT

CONFIG += c++20

win32 {
    #enable c++20 explicitly in msvc
    QMAKE_CXXFLAGS += /std:c++20 /Zc:__cplusplus /permissive-
}

DEFINES += NOMINMAX

if(unix|mingw):QMAKE_CXXFLAGS_RELEASE += -DNDEBUG
win32:msvc:QMAKE_CXXFLAGS_RELEASE += /DNDEBUG

# check Qt version
defineTest(minQtVersion) {
  maj = $$1
  min = $$2
  patch = $$3
  isEqual(QT_MAJOR_VERSION, $$maj) {
    isEqual(QT_MINOR_VERSION, $$min) {
      isEqual(QT_PATCH_VERSION, $$patch) {
        return(true)
      }
      greaterThan(QT_PATCH_VERSION, $$patch) {
        return(true)
      }
    }
    greaterThan(QT_MINOR_VERSION, $$min) {
      return(true)
    }
  }
  greaterThan(QT_MAJOR_VERSION, $$maj) {
    return(true)
  }
  return(false)
}

!minQtVersion(5, 15, 0) {
  error(YACReader requires Qt 5.15 or newer but $$[QT_VERSION] was detected)
}

# reduce log pollution
CONFIG += silent

# Disable coverflow for arm targets
isEmpty(QMAKE_TARGET.arch) {
  QMAKE_TARGET.arch = $$QMAKE_HOST.arch
}

# default value for comic archive decompression backend
unix:!macx:!CONFIG(unarr):!CONFIG(7zip):!CONFIG(libarchive) {
  CONFIG += unarr
}

win32:!CONFIG(unarr):!CONFIG(7zip):!CONFIG(libarchive) {
  CONFIG += 7zip
}

macx:!CONFIG(unarr):!CONFIG(7zip):!CONFIG(libarchive) {
  CONFIG += 7zip
}

# default values for pdf render backend
win32:!CONFIG(poppler):!CONFIG(pdfium):!CONFIG(no_pdf) {
  CONFIG += pdfium
}

unix:!macx:!CONFIG(poppler):!CONFIG(pdfium):!CONFIG(no_pdf) {
  CONFIG += poppler
}

macx:!CONFIG(pdfkit):!CONFIG(pdfium):!CONFIG(no_pdf) {
  CONFIG += pdfkit
}

!CONFIG(poppler) {
    DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050F00
} else {
    DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050900
}

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
}

DEFINES += QT_DEPRECATED_WARNINGS
