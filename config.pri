# functions to automatically initialize some of YACReader's build options to
# default values if they're not set on build time
# for a more detailed description, see INSTALL.TXT

CONFIG += c++11

unix:QMAKE_CXXFLAGS_RELEASE += -DNDEBUG
win32:QMAKE_CXXFLAGS_RELEASE += /DNDEBUG

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

!minQtVersion(5, 9, 0) {
  error(YACReader requires Qt 5.9 or newer but $$[QT_VERSION] was detected)
}

minQtVersion(6, 0, 0) {
  error(YACReader does not support building with Qt6 (yet))
}

# reduce log pollution
CONFIG += silent

# Disable coverflow for arm targets
isEmpty(QMAKE_TARGET.arch) {
  QMAKE_TARGET.arch = $$QMAKE_HOST.arch
}
contains(QMAKE_TARGET.arch, arm.*)|contains(QMAKE_TARGET.arch, aarch.*) {
  message("Building for ARM architecture. Disabling OpenGL coverflow ...")
  CONFIG += no_opengl
}

# build without opengl widget support
CONFIG(no_opengl) {
  DEFINES += NO_OPENGL
}

# default value for comic archive decompression backend
unix:!macx:!CONFIG(unarr):!CONFIG(7zip) {
  CONFIG += unarr
}

win32:!CONFIG(unarr):!CONFIG(7zip) {
  CONFIG += 7zip
}

macx:!CONFIG(unarr):!CONFIG(7zip) {
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
