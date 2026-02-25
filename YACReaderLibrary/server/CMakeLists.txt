# YACReaderLibrary server (REST API + HTTP server)

add_library(server STATIC
    static.h
    static.cpp
    requestmapper.h
    requestmapper.cpp
    yacreader_http_server.h
    yacreader_http_server.cpp
    yacreader_http_session.h
    yacreader_http_session.cpp
    yacreader_http_session_store.h
    yacreader_http_session_store.cpp
    yacreader_server_data_helper.h
    yacreader_server_data_helper.cpp
    controllers/versioncontroller.h
    controllers/versioncontroller.cpp
    controllers/v2/comiccontroller_v2.h
    controllers/v2/comiccontroller_v2.cpp
    controllers/v2/errorcontroller_v2.h
    controllers/v2/errorcontroller_v2.cpp
    controllers/v2/folderinfocontroller_v2.h
    controllers/v2/folderinfocontroller_v2.cpp
    controllers/v2/librariescontroller_v2.h
    controllers/v2/librariescontroller_v2.cpp
    controllers/v2/pagecontroller_v2.h
    controllers/v2/pagecontroller_v2.cpp
    controllers/v2/covercontroller_v2.h
    controllers/v2/covercontroller_v2.cpp
    controllers/v2/updatecomiccontroller_v2.h
    controllers/v2/updatecomiccontroller_v2.cpp
    controllers/v2/comicdownloadinfocontroller_v2.h
    controllers/v2/comicdownloadinfocontroller_v2.cpp
    controllers/v2/synccontroller_v2.h
    controllers/v2/synccontroller_v2.cpp
    controllers/v2/foldercontentcontroller_v2.h
    controllers/v2/foldercontentcontroller_v2.cpp
    controllers/v2/tagscontroller_v2.h
    controllers/v2/tagscontroller_v2.cpp
    controllers/v2/tagcontentcontroller_v2.h
    controllers/v2/tagcontentcontroller_v2.cpp
    controllers/v2/favoritescontroller_v2.h
    controllers/v2/favoritescontroller_v2.cpp
    controllers/v2/readingcomicscontroller_v2.h
    controllers/v2/readingcomicscontroller_v2.cpp
    controllers/v2/readinglistscontroller_v2.h
    controllers/v2/readinglistscontroller_v2.cpp
    controllers/v2/readinglistcontentcontroller_v2.h
    controllers/v2/readinglistcontentcontroller_v2.cpp
    controllers/v2/comicfullinfocontroller_v2.h
    controllers/v2/comicfullinfocontroller_v2.cpp
    controllers/v2/readinglistinfocontroller_v2.h
    controllers/v2/readinglistinfocontroller_v2.cpp
    controllers/v2/taginfocontroller_v2.h
    controllers/v2/taginfocontroller_v2.cpp
    controllers/v2/comiccontrollerinreadinglist_v2.h
    controllers/v2/comiccontrollerinreadinglist_v2.cpp
    controllers/v2/foldermetadatacontroller_v2.h
    controllers/v2/foldermetadatacontroller_v2.cpp
    controllers/v2/searchcontroller_v2.h
    controllers/v2/searchcontroller_v2.cpp
    controllers/webui/statuspagecontroller.h
    controllers/webui/statuspagecontroller.cpp
)
target_include_directories(server PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers/v2
)
target_compile_definitions(server PUBLIC SERVER_VERSION_NUMBER="2.1")
if(UNIX AND NOT APPLE)
    target_compile_definitions(server PRIVATE
        "DATADIR=\"${CMAKE_INSTALL_FULL_DATADIR}\"")
endif()
target_link_libraries(server PUBLIC
    Qt::Core
    Qt::Network
    Qt::Sql
    QtWebApp_httpserver
    QtWebApp_templateengine
    QsLog
    common_all
    comic_backend
    db_helper
)
