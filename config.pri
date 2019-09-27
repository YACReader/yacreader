# functions to automatically initialize some of YACReader's build options to
# default values if they're not set on build time
# for a more detailed description, see INSTALL.TXT

# check Qt version
QT_VERSION = $$[QT_VERSION]
QT_VERSION = $$split(QT_VERSION, ".")
QT_VER_MAJ = $$member(QT_VERSION, 0)
QT_VER_MIN = $$member(QT_VERSION, 1)

lessThan(QT_VER_MAJ, 5) {
error(YACReader requires Qt 5 or newer but Qt $$[QT_VERSION] was detected.)
  }
lessThan(QT_VER_MIN, 6):!CONFIG(no_opengl) {
  error ("You need at least Qt 5.6 to compile YACReader or YACReaderLibrary.")
  }

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
  CONFIG += pdfium
}
