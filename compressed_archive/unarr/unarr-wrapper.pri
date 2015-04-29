INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += $$PWD/extract_delegate.h \
			$$PWD/compressed_archive.h \

SOURCES += $$PWD/compressed_archive.cpp \

unix:!macx:exists (/usr/include/unarr.h) {
		message(Using system provided unarr installation)
		LIBS+=-lunarr
		}
else:macx:exists (../../dependencies/unarr/libunarr.dynlib) {
		LIBS += -L../../dependencies/unarr/ -lunarr
		}

else:win32:exists (../../dependencies/unarr/unarr.dll) {
		LIBS += -L../../dependencies/unarr/ -lunarr
		}

else:exists ($$PWD/unarr-master) {
		message(Found unarr source-code)
		message(Unarr will be build as a part of YACReader)
		
		#qmake based unarr build system
		#this should only be used for testing or as a last resort
		include(unarr.pro)
		}
	else {
		error(Missing dependency: unarr decrompression backend. Please install libunarr on your system\
		or provide a copy of the unarr source code in compressed_archive/unarr/unarr-master)
		}