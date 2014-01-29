INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

CONFIG   += precompile_header

win32 {PRECOMPILED_HEADER = $$PWD/StdAfx.h}
!win32 {PRECOMPILED_HEADER = $$PWD/libp7zip/CPP/myWindows/StdAfx.h}

win32 {
INCLUDEPATH  += $$PWD/lib7zip/CPP/

DEFINES += _UNICODE _WIN32 

SOURCES += $$PWD/compressed_archive.cpp \
    $$PWD/lib7zip/CPP/Windows/FileIO.cpp \
    $$PWD/lib7zip/CPP/Windows/PropVariant.cpp \
    $$PWD/lib7zip/CPP/Windows/PropVariantConversions.cpp \
    $$PWD/lib7zip/CPP/Common/IntToString.cpp \
    $$PWD/lib7zip/CPP/Common/MyString.cpp \
    $$PWD/lib7zip/CPP/Common/MyVector.cpp \
    $$PWD/lib7zip/CPP/Common/StringConvert.cpp \
    $$PWD/lib7zip/CPP/Common/Wildcard.cpp \
    $$PWD/lib7zip/CPP/7zip/Common/FileStreams.cpp \
	$$PWD/lib7zip/CPP/7zip/Common/StreamUtils.cpp \
	$$PWD/lib7zip/C/Alloc.c \
	$$PWD/lib7zip/CPP/7zip/Common/StreamObjects.cpp

HEADERS += $$PWD/compressed_archive.h \
    $$PWD/extract_delegate.h \
    $$PWD/7z_includes.h \
	$$PWD/open_callbacks.h \
	$$PWD/extract_callbacks.h\
	$$PWD/lib7zip/CPP/Windows/FileIO.h \
	$$PWD/lib7zip/CPP/Windows/PropVariant.h \
	$$PWD/lib7zip/CPP/Windows/PropVariantConversions.h \
	$$PWD/lib7zip/CPP/Common/IntToString.h \
	$$PWD/lib7zip/CPP/Common/MyString.h \
	$$PWD/lib7zip/CPP/Common/MyVector.h \
	$$PWD/lib7zip/CPP/Common/StringConvert.h \
	$$PWD/lib7zip/CPP/Common/Wildcard.h \
	$$PWD/lib7zip/CPP/7zip/Common/FileStreams.h \
	$$PWD/lib7zip/CPP/7zip/IStream.h \
	$$PWD/lib7zip/CPP/7zip/Common/StreamUtils.h \
	$$PWD/lib7zip/C/Alloc.h \
	$$PWD/lib7zip/CPP/7zip/Common/StreamObjects.h
}

macx{
LIBS += -framework IOKit -framework CoreFoundation

DEFINES += UNICODE _UNICODE _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE \
    NDEBUG _REENTRANT ENV_UNIX \
    _7ZIP_LARGE_PAGES ENV_MACOSX _TCHAR_DEFINED
}

unix:!macx{
DEFINES += _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE \
	NDEBUG _REENTRANT ENV_UNIX \
	_7ZIP_LARGE_PAGES
	}

!win32 {
INCLUDEPATH  += $$PWD/libp7zip/CPP/ \
                $$PWD/libp7zip/CPP/myWindows/ \
                $$PWD/libp7zip/CPP/include_windows/
	
SOURCES += $$PWD/compressed_archive.cpp \
    $$PWD/libp7zip/CPP/Windows/FileIO.cpp \
    $$PWD/libp7zip/CPP/Windows/FileFind.cpp \
    $$PWD/libp7zip/CPP/Windows/PropVariant.cpp \
    $$PWD/libp7zip/CPP/Windows/PropVariantConversions.cpp \
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

HEADERS += $$PWD/compressed_archive.h \
    $$PWD/7z_includes.h \
	$$PWD/open_callbacks.h \
	$$PWD/extract_callbacks.h\
	$$PWD/libp7zip/CPP/include_windows/windows.h \
	$$PWD/libp7zip/CPP/include_windows/tchar.h \
	$$PWD/libp7zip/CPP/include_windows/basetyps.h \
	$$PWD/libp7zip/CPP/Windows/FileFind.h \
	$$PWD/libp7zip/CPP/Windows/FileIO.h \
	$$PWD/libp7zip/CPP/Windows/PropVariant.h \
	$$PWD/libp7zip/CPP/Windows/PropVariantConversions.h \
	$$PWD/libp7zip/CPP/Common/IntToString.h \
	$$PWD/libp7zip/CPP/Common/MyString.h \
	$$PWD/libp7zip/CPP/Common/MyVector.h \
	$$PWD/libp7zip/CPP/Common/StringConvert.h \
	$$PWD/libp7zip/CPP/Common/Wildcard.h \
	$$PWD/libp7zip/CPP/7zip/Common/FileStreams.h \
	$$PWD/libp7zip/CPP/7zip/IStream.h \
	$$PWD/libp7zip/CPP/7zip/Common/StreamUtils.h \
	$$PWD/libp7zip/C/Alloc.h \
	$$PWD/libp7zip/CPP/7zip/Common/StreamObjects.h \
        $$PWD/libp7zip/CPP/Common/MyWindows.h \
        $$PWD/libp7zip/CPP/7zip/ICoder.h \
}


	
