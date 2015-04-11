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
	#these flags are probably redundant
	#qmake tends to chose it's own flags and ignores system wide flags on unix
	QMAKE_CXXFLAGS+= -O2 -pipe -Wall -pthread -std=c++11
	QMAKE_CFLAGS+= -O2 -pipe -Wall -pthread
	
	#this might need a little tweaking on mac os
	!CONFIG(unarr){
		DEFINES += "LIBDIR=\\\"$$/usr/lib\\\""
		}
		else{
		#make unarr use system zlib and bzip2
		#these are pretty much standard on unix
		DEFINES += HAVE_ZLIB HAVE_BZIP2
		#add zlib and bz2 to libs
		LIBS += -lbz2 -lz
		}
	}

!CONFIG(unarr){
    include(../../compressed_archive/wrapper.pri)
} else {
    include(../../compressed_archive/unarr/unarr-wrapper.pri)
}



