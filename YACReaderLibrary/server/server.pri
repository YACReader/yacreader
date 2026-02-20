INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/controllers
INCLUDEPATH += $$PWD/controllers/v2
DEPENDPATH += $$PWD
DEPENDPATH += $$PWD/controllers
DEPENDPATH += $$PWD/controllers/v2


HEADERS += \
    $$PWD/controllers/v2/foldermetadatacontroller_v2.h \
    $$PWD/controllers/v2/searchcontroller_v2.h \
    $$PWD/static.h \
    $$PWD/requestmapper.h \
    $$PWD/yacreader_http_server.h \
    $$PWD/yacreader_http_session.h \
    $$PWD/yacreader_http_session_store.h \
    $$PWD/yacreader_server_data_helper.h \
    $$PWD/controllers/versioncontroller.h \
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
    $$PWD/controllers/v2/comiccontrollerinreadinglist_v2.h\
    #Browser
    $$PWD/controllers/webui/statuspagecontroller.h


SOURCES += \
    $$PWD/controllers/v2/foldermetadatacontroller_v2.cpp \
    $$PWD/controllers/v2/searchcontroller_v2.cpp \
    $$PWD/static.cpp \
    $$PWD/requestmapper.cpp \
    $$PWD/yacreader_http_server.cpp \
    $$PWD/yacreader_http_session.cpp \
    $$PWD/yacreader_http_session_store.cpp \
    $$PWD/yacreader_server_data_helper.cpp \
    $$PWD/controllers/versioncontroller.cpp \
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
    $$PWD/controllers/v2/comiccontrollerinreadinglist_v2.cpp \
    #WebUI
    $$PWD/controllers/webui/statuspagecontroller.cpp

include(../../third_party/QtWebApp/httpserver/httpserver.pri)
include(../../third_party/QtWebApp/templateengine/templateengine.pri)

DEFINES += SERVER_VERSION_NUMBER=\\\"2.1\\\"
