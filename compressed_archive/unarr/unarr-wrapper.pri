INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += $$PWD/extract_delegate.h \
			$$PWD/compressed_archive.h \

SOURCES += $$PWD/compressed_archive.cpp \

unix:!macx {
		message(Using system provided unarr installation)
		CONFIG += link_pkgconfig
		PKGCONFIG += libunarr
		DEFINES += use_unarr
		}
else:macx:exists (../../dependencies/unarr/macx/libunarr.a) {
		message(Found prebuilt unarr library in dependencies directory.)
		INCLUDEPATH += $$PWD/../../dependencies/unarr/macx
		LIBS += -L$$PWD/../../dependencies/unarr/macx -lunarr -lz -lbz2
		DEFINES+=use_unarr
		}

else:win32:exists (../../dependencies/unarr/win/unarr.h) {
                message(Found prebuilt unarr library in dependencies directory.)
                INCLUDEPATH += $$PWD/../../dependencies/unarr/win
                contains(QMAKE_TARGET.arch, x86_64): {
                        LIBS += -L$$PWD/../../dependencies/unarr/win/x64 -lunarr
                } else {
                        LIBS += -L$$PWD/../../dependencies/unarr/win/x86 -lunarr
                }
                DEFINES+=use_unarr UNARR_IS_SHARED_LIBRARY
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
