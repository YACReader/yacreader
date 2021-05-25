INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/controllers
INCLUDEPATH += $$PWD/controllers/v1
INCLUDEPATH += $$PWD/controllers/v2
DEPENDPATH += $$PWD
DEPENDPATH += $$PWD/controllers
DEPENDPATH += $$PWD/controllers/v1
DEPENDPATH += $$PWD/controllers/v2


HEADERS += \
    $$PWD/static.h \
    $$PWD/startup.h \
    $$PWD/requestmapper.h \
    $$PWD/yacreader_http_session.h \
    $$PWD/yacreader_http_session_store.h \
    $$PWD/yacreader_server_data_helper.h \
    $$PWD/controllers/versioncontroller.h \
    #v1
    $$PWD/controllers/v1/comiccontroller.h \
    $$PWD/controllers/v1/errorcontroller.h \
    $$PWD/controllers/v1/foldercontroller.h \
    $$PWD/controllers/v1/folderinfocontroller.h \
    $$PWD/controllers/v1/librariescontroller.h \
    $$PWD/controllers/v1/pagecontroller.h \
    $$PWD/controllers/v1/covercontroller.h \
    $$PWD/controllers/v1/updatecomiccontroller.h \
    $$PWD/controllers/v1/comicdownloadinfocontroller.h \
    $$PWD/controllers/v1/synccontroller.h \
    #v2
    $$PWD/controllers/v2/comiccontroller_v2.h \
    $$PWD/controllers/v2/errorcontroller_v2.h \
    $$PWD/controllers/v2/folderinfocontroller_v2.h \
    $$PWD/controllers/v2/librariescontroller_v2.h \
    $$PWD/controllers/v2/pagecontroller_v2.h \
    $$PWD/controllers/v2/covercontroller_v2.h \
    $$PWD/controllers/v2/updatecomiccontroller_v2.h \
    $$PWD/controllers/v2/comicdownloadinfocontroller_v2.h \
    $$PWD/controllers/v2/synccontroller_v2.h \
    $$PWD/controllers/v2/foldercontentcontroller_v2.h \
    $$PWD/controllers/v2/tagscontroller_v2.h \
    $$PWD/controllers/v2/tagcontentcontroller_v2.h \
    $$PWD/controllers/v2/favoritescontroller_v2.h \
    $$PWD/controllers/v2/readingcomicscontroller_v2.h \
    $$PWD/controllers/v2/readinglistscontroller_v2.h \
    $$PWD/controllers/v2/readinglistcontentcontroller_v2.h \
    $$PWD/controllers/v2/comicfullinfocontroller_v2.h \
    $$PWD/controllers/v2/readinglistinfocontroller_v2.h \
    $$PWD/controllers/v2/taginfocontroller_v2.h \
    $$PWD/controllers/v2/comiccontrollerinreadinglist_v2.h


SOURCES += \
    $$PWD/static.cpp \
    $$PWD/startup.cpp \
    $$PWD/requestmapper.cpp \
    $$PWD/yacreader_http_session.cpp \
    $$PWD/yacreader_http_session_store.cpp \
    $$PWD/yacreader_server_data_helper.cpp \
    $$PWD/controllers/versioncontroller.cpp \
    #v1
    $$PWD/controllers/v1/comiccontroller.cpp \
    $$PWD/controllers/v1/errorcontroller.cpp \
    $$PWD/controllers/v1/foldercontroller.cpp \
    $$PWD/controllers/v1/folderinfocontroller.cpp \
    $$PWD/controllers/v1/librariescontroller.cpp \
    $$PWD/controllers/v1/pagecontroller.cpp \
    $$PWD/controllers/v1/covercontroller.cpp \
    $$PWD/controllers/v1/updatecomiccontroller.cpp \
    $$PWD/controllers/v1/comicdownloadinfocontroller.cpp \
    $$PWD/controllers/v1/synccontroller.cpp \
    #v2
    $$PWD/controllers/v2/comiccontroller_v2.cpp \
    $$PWD/controllers/v2/errorcontroller_v2.cpp \
    $$PWD/controllers/v2/folderinfocontroller_v2.cpp \
    $$PWD/controllers/v2/librariescontroller_v2.cpp \
    $$PWD/controllers/v2/pagecontroller_v2.cpp \
    $$PWD/controllers/v2/covercontroller_v2.cpp \
    $$PWD/controllers/v2/updatecomiccontroller_v2.cpp \
    $$PWD/controllers/v2/comicdownloadinfocontroller_v2.cpp \
    $$PWD/controllers/v2/synccontroller_v2.cpp \
    $$PWD/controllers/v2/foldercontentcontroller_v2.cpp \
    $$PWD/controllers/v2/tagscontroller_v2.cpp \
    $$PWD/controllers/v2/tagcontentcontroller_v2.cpp \
    $$PWD/controllers/v2/favoritescontroller_v2.cpp \
    $$PWD/controllers/v2/readingcomicscontroller_v2.cpp \
    $$PWD/controllers/v2/readinglistscontroller_v2.cpp \
    $$PWD/controllers/v2/readinglistcontentcontroller_v2.cpp \
    $$PWD/controllers/v2/comicfullinfocontroller_v2.cpp \
    $$PWD/controllers/v2/readinglistinfocontroller_v2.cpp \
    $$PWD/controllers/v2/taginfocontroller_v2.cpp \
    $$PWD/controllers/v2/comiccontrollerinreadinglist_v2.cpp

include(../../third_party/QtWebApp/httpserver/httpserver.pri)
include(../../third_party/QtWebApp/templateengine/templateengine.pri)

DEFINES += SERVER_VERSION_NUMBER=\\\"2.0\\\"
