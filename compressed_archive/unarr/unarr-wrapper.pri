INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += $$PWD/extract_delegate.h \
			$$PWD/compressed_archive.h \

SOURCES += $$PWD/compressed_archive.cpp \

#static build (no lib)
include(unarr.pro)

#for system libunarr.so
#LIBS+=-lunarr