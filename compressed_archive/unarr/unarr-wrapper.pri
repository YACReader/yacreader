INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += $$PWD/extract_delegate.h \
			$$PWD/compressed_archive.h \

SOURCES += $$PWD/compressed_archive.cpp \

unix:!macx:exists (/usr/include/unarr.h) {
		message(Using system provided unarr installation)
		LIBS+=-lunarr
		DEFINES+=use_unarr
		}
else:macx:exists (../../dependencies/unarr/libunarr.a) {
		message(Found prebuilt unarr library.)
		INCLUDEPATH += $$PWD/../../dependencies/unarr
		LIBS += -L$$PWD/../../dependencies/unarr/ -lunarr -lz -lbz2
		DEFINES+=use_unarr
		}

else:win32:exists (../../dependencies/unarr/unarr.dll) {
		LIBS += -L../../dependencies/unarr/ -lunarr
		DEFINES+=use_unarr
		}

else:exists ($$PWD/unarr-master) {
		message(Found unarr source-code)
		message(Unarr will be build as a part of YACReader)
		
		#qmake based unarr build system
		#this should only be used for testing or as a last resort
		include(unarr.pro)
		DEFINES+=use_unarr
		}
	else {
		error(Missing dependency: unarr decrompression backend. Please install libunarr on your system\
		or provide a copy of the unarr source code in compressed_archive/unarr/unarr-master)
		}
