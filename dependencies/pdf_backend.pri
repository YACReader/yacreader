CONFIG(no_pdf) {
  DEFINES += "NO_PDF"
}

CONFIG(pdfium) {
  DEFINES += "USE_PDFIUM"
  SOURCES += ../common/pdf_comic.cpp
  win32 {
    INCLUDEPATH += $$PWD/pdfium/win/public
    contains(QMAKE_TARGET.arch, x86_64): {
      LIBS += -L$$PWD/pdfium/win/x64 -lpdfium
    } else {
      LIBS += -L$$PWD/pdfium/win/x86 -lpdfium
    }
  }
  unix {
    macx {
      LIBS += -L$$PWD/pdfium/macx/bin -lpdfium
      INCLUDEPATH += $$PWD/pdfium/macx/include
    }
    else:!contains(QT_CONFIG, no-pkg-config):packagesExist(libpdfium) {
      message(Using system provided installation of libpdfium found by pkg-config.)
      CONFIG += link_pkgconfig
      PKGCONFIG += libpdfium
    } else:exists(/usr/include/pdfium) {
      message(Using libpdfium found at /usr/lib/pdfium)
      INCLUDEPATH += /usr/include/pdfium
      LIBS += -lpdfium
    } else {
      error(Could not find libpdfium.)
    }
  }
}

CONFIG(pdfkit) {
  !macx {
    error (Pdfkit is macOS only)
  } else {
    DEFINES += "USE_PDFKIT"
    OBJECTIVE_SOURCES += ../common/pdf_comic.mm
  }
}

CONFIG(poppler) {
  win32 {
    contains(QMAKE_TARGET.arch, x86_64): {
    error ("We currently don't ship precompiled poppler libraries for 64 bit builds on Windows")
    }
    INCLUDEPATH += $$PWD/poppler/include/qt5
    LIBS += -L$$PWD/poppler/lib -lpoppler-qt5
    # Add extra paths for dll dependencies so the executables don't crash when launching
    # from QtCreator
    LIBS += -L$$PWD/poppler/bin
    LIBS += -L$$PWD/poppler/dependencies/bin
  }
  unix:!macx {
    !contains(QT_CONFIG, no-pkg-config):packagesExist(poppler-qt5) {
      message("Using system provided installation of poppler-qt5 found by pkg-config.")
      CONFIG += link_pkgconfig
      PKGCONFIG += poppler-qt5
    } else:!macx:exists(/usr/include/poppler/qt5) {
      message("Using system provided installation of poppler-qt5.")
      INCLUDEPATH  += /usr/include/poppler/qt5
      LIBS += -lpoppler-qt5
    } else {
      error("Could not find poppler-qt5")
    }

  }
  unix:macx {
    error (Poppler backend is currently not supported on macOS)
  }
}
