INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

exists (../compressed_archive/lib7zip) {
    message(Using 7zip)
} else {
    error(You\'ll need 7zip source code to compile YACReader. \
          Please check the compressed_archive folder for further instructions.)
}

INCLUDEPATH += \
    $$PWD/lib7zip/CPP \

SOURCES += \
    $$PWD/compressed_archive.cpp \
    $$PWD/lib7zip/CPP/Windows/FileIO.cpp \
    $$PWD/lib7zip/CPP/Windows/PropVariant.cpp \
    $$PWD/lib7zip/CPP/Windows/PropVariantConv.cpp \
    $$PWD/lib7zip/CPP/Common/IntToString.cpp \
    $$PWD/lib7zip/CPP/Common/MyString.cpp \
    $$PWD/lib7zip/CPP/Common/MyVector.cpp \
    $$PWD/lib7zip/CPP/Common/Wildcard.cpp \
    $$PWD/lib7zip/CPP/7zip/Common/FileStreams.cpp \
    $$PWD/lib7zip/C/Alloc.c \
    $$PWD/lib7zip/CPP/7zip/Common/StreamObjects.cpp

unix{
SOURCES += \
    $$PWD/lib7zip/CPP/Common/NewHandler.cpp \
    $$PWD/lib7zip/CPP/Windows/DLL.cpp \
    $$PWD/lib7zip/CPP/Windows/FileDir.cpp \
    $$PWD/lib7zip/CPP/Windows/FileFind.cpp \
    $$PWD/lib7zip/CPP/Windows/FileName.cpp \
    $$PWD/lib7zip/CPP/Windows/TimeUtils.cpp \
    $$PWD/lib7zip/CPP/Common/UTFConvert.cpp \
    $$PWD/lib7zip/CPP/Common/MyWindows.cpp \
    $$PWD/lib7zip/CPP/Common/StringConvert.cpp \
}

HEADERS += \
    $$PWD/lib7zip/CPP/Common/Common.h \
    $$PWD/compressed_archive.h \
    $$PWD/extract_delegate.h \
    $$PWD/7z_includes.h \
    $$PWD/open_callbacks.h \
    $$PWD/extract_callbacks.h \

macx{
LIBS += -framework IOKit -framework CoreFoundation

DEFINES += _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE \
    NDEBUG _REENTRANT ENV_UNIX \
    _7ZIP_LARGE_PAGES ENV_MACOSX _TCHAR_DEFINED \
    UNICODE _UNICODE UNIX_USE_WIN_FILE
}

unix:!macx{
DEFINES += _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE \
    NDEBUG _REENTRANT ENV_UNIX \
    _7ZIP_LARGE_PAGES \
    UNICODE _UNICODE UNIX_USE_WIN_FILE
}
