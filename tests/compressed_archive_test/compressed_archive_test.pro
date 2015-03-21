TEMPLATE = app
CONFIG += console

SOURCES += \
    main.cpp \

win32 {
    LIBS +=  -loleaut32 -lole32
    QMAKE_CXXFLAGS_RELEASE += /MP /Ob2 /Oi /Ot /GT
    QMAKE_LFLAGS_RELEASE += /LTCG
    CONFIG -= embed_manifest_exe
}

include(../../compressed_archive/wrapper.pri)



