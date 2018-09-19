INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

win32 {
!exists (../compressed_archive/lib7zip) {
	error(You\'ll need 7zip source code to compile YACReader. \
	Please check the compressed_archive folder for further instructions.)
}
}

unix {
exists (../compressed_archive/libp7zip) {
	message(Found p7zip source code...)
        #this is probably only needed in macos
	system(patch -N -p0 -i libp7zip.patch)
} else {
	error(You\'ll need 7zip source code to compile YACReader. \
	Please check the compressed_archive folder for further instructions.)
}
}

win32 {
INCLUDEPATH += \
    $$PWD/lib7zip/myWindows \
    $$PWD/lib7zip/CPP \
    $$PWD/lib7zip/include_windows \

DEFINES += _UNICODE _WIN32

SOURCES += \
    $$PWD/compressed_archive.cpp \
    $$PWD/lib7zip/CPP/Windows/FileIO.cpp \
    $$PWD/lib7zip/CPP/Windows/PropVariant.cpp \
    $$PWD/lib7zip/CPP/Windows/PropVariantConv.cpp \
    $$PWD/lib7zip/CPP/Common/IntToString.cpp \
    $$PWD/lib7zip/CPP/Common/MyString.cpp \
    $$PWD/lib7zip/CPP/Common/MyVector.cpp \
    $$PWD/lib7zip/CPP/Common/StringConvert.cpp \
    $$PWD/lib7zip/CPP/Common/Wildcard.cpp \
    $$PWD/lib7zip/CPP/7zip/Common/FileStreams.cpp \
    $$PWD/lib7zip/CPP/7zip/Common/StreamUtils.cpp \
    $$PWD/lib7zip/C/Alloc.c \
    $$PWD/lib7zip/CPP/7zip/Common/StreamObjects.cpp

HEADERS += \
    $$PWD/lib7zip/CPP/Common/Common.h \
    $$PWD/compressed_archive.h \
    $$PWD/extract_delegate.h \
    $$PWD/7z_includes.h \
    $$PWD/open_callbacks.h \
    $$PWD/extract_callbacks.h
}

!win32 {
CONFIG   += precompile_header
PRECOMPILED_HEADER = $$PWD/7z_precompiled_header.h
}

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

!win32 {
INCLUDEPATH  += $$PWD/libp7zip/CPP/myWindows/ \
                $$PWD/libp7zip/CPP/ \
                $$PWD/libp7zip/CPP/include_windows/

SOURCES += \
    $$PWD/compressed_archive.cpp \
    $$PWD/libp7zip/CPP/Windows/PropVariant.cpp \
    $$PWD/libp7zip/CPP/Windows/PropVariantConv.cpp \
    $$PWD/libp7zip/CPP/Windows/FileIO.cpp \
    $$PWD/libp7zip/CPP/Windows/FileFind.cpp \
    $$PWD/libp7zip/CPP/Common/IntToString.cpp \
    $$PWD/libp7zip/CPP/Common/MyString.cpp \
    $$PWD/libp7zip/CPP/Common/MyVector.cpp \
    $$PWD/libp7zip/CPP/Common/StringConvert.cpp \
    $$PWD/libp7zip/CPP/Common/Wildcard.cpp \
    $$PWD/libp7zip/CPP/7zip/Common/FileStreams.cpp \
    $$PWD/libp7zip/CPP/7zip/Common/StreamUtils.cpp \
    $$PWD/libp7zip/C/Alloc.c \
    $$PWD/libp7zip/CPP/7zip/Common/StreamObjects.cpp \
    $$PWD/libp7zip/CPP/myWindows/wine_date_and_time.cpp \
    $$PWD/libp7zip/CPP/Common/MyWindows.cpp

HEADERS += \
    $$PWD/compressed_archive.h \
    $$PWD/7z_includes.h
}	
