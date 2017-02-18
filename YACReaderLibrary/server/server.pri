INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/static.h \
    $$PWD/startup.h \
    $$PWD/requestmapper.h \
    $$PWD/controllers/comiccontroller.h \
    $$PWD/controllers/errorcontroller.h \
    $$PWD/controllers/foldercontroller.h \
    $$PWD/controllers/folderinfocontroller.h \
    $$PWD/controllers/librariescontroller.h \
    $$PWD/controllers/pagecontroller.h \
    $$PWD/controllers/sessionmanager.h \
    $$PWD/controllers/covercontroller.h \
    $$PWD/controllers/updatecomiccontroller.h \
    $$PWD/controllers/comicdownloadinfocontroller.h \
    $$PWD/controllers/synccontroller.h \
    #v2
    $$PWD/controllers/versioncontroller.h \
    $$PWD/controllers/foldercontentcontroller.h \
    $$PWD/controllers/tagscontroller.h \
    $$PWD/yacreader_http_session.h \
    $$PWD/yacreader_http_session_store.h \
    $$PWD/controllers/tagcontentcontroller.h \
    $$PWD/yacreader_server_data_helper.h \
    $$PWD/controllers/favoritescontroller.h \
    $$PWD/controllers/readingcomicscontroller.h \
    $$PWD/controllers/readinglistscontroller.h

SOURCES += \
    $$PWD/static.cpp \
    $$PWD/startup.cpp \
    $$PWD/requestmapper.cpp \
    $$PWD/controllers/comiccontroller.cpp \
    $$PWD/controllers/errorcontroller.cpp \
    $$PWD/controllers/foldercontroller.cpp \
    $$PWD/controllers/folderinfocontroller.cpp \
    $$PWD/controllers/librariescontroller.cpp \
    $$PWD/controllers/pagecontroller.cpp \
    $$PWD/controllers/sessionmanager.cpp \
    $$PWD/controllers/covercontroller.cpp \
    $$PWD/controllers/updatecomiccontroller.cpp \
    $$PWD/controllers/comicdownloadinfocontroller.cpp \
    $$PWD/controllers/synccontroller.cpp \
    #v2
    $$PWD/controllers/versioncontroller.cpp \
    $$PWD/controllers/foldercontentcontroller.cpp \
    $$PWD/controllers/tagscontroller.cpp \
    $$PWD/yacreader_http_session.cpp \
    $$PWD/yacreader_http_session_store.cpp \
    $$PWD/controllers/tagcontentcontroller.cpp \
    $$PWD/yacreader_server_data_helper.cpp \
    $$PWD/controllers/favoritescontroller.cpp \
    $$PWD/controllers/readingcomicscontroller.cpp \
    $$PWD/controllers/readinglistscontroller.cpp
	
include(lib/logging/logging.pri)
include(lib/httpserver/httpserver.pri)
include(lib/templateengine/templateengine.pri)

DEFINES += SERVER_VERSION_NUMBER=\\\"2.0\\\"
