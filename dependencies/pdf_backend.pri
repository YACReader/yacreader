CONFIG(no_pdf) {
	DEFINES += "NO_PDF"
}

CONFIG(pdfium) {
	DEFINES += "USE_PDFIUM"
	SOURCES += ../common/pdf_comic.cpp
	win32 {
		INCLUDEPATH += ../dependencies/pdfium/win/public
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
			!macx {
				INCLUDEPATH += /usr/include/pdfium
				LIBS += -L/usr/lib/pdfium -lpdfium -lfreetype
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
		LIBS += -Lpoppler/lib -lpoppler-qt5
		INCLUDEPATH += poppler/include/qt5
	}
	unix:!macx {
		INCLUDEPATH  += /usr/include/poppler/qt5
	    	LIBS += -L/usr/lib -lpoppler-qt5
	}
	unix:macx {
		error (Poppler backend is currently not supported on macOS)
	}
}
