TEMPLATE = app
CONFIG += console

SOURCES += \
    main.cpp \

QT += core

win32 {
    LIBS +=  -loleaut32 -lole32
    QMAKE_CXXFLAGS_RELEASE += /MP /Ob2 /Oi /Ot /GT
    QMAKE_LFLAGS_RELEASE += /LTCG
    CONFIG -= embed_manifest_exe
}

unix {

	QMAKE_CXXFLAGS+= -O3 -pipe -Wall -pthread -std=c++11
	QMAKE_CFLAGS+= -O3 -pipe -Wall -pthread
	
	#this might need a little tweaking on mac os
	DEFINES += "LIBDIR=\\\"$$/usr/lib\\\"" HAVE_ZLIB HAVE_BZIP2
	
	LIBS += -lbz2 -lz
	}

!CONFIG(unarr){
    include(../../compressed_archive/wrapper.pri)
} else {
    include(../../compressed_archive/unarr/unarr-wrapper.pri)
}



