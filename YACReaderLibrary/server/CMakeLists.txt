add_library(
  server
  # Legacy controller
  controllers/v1/comiccontroller.cpp
  controllers/v1/comiccontroller.h
  controllers/v1/comicdownloadinfocontroller.cpp
  controllers/v1/comicdownloadinfocontroller.h
  controllers/v1/covercontroller.cpp
  controllers/v1/covercontroller.h
  controllers/v1/errorcontroller.cpp
  controllers/v1/errorcontroller.h
  controllers/v1/foldercontroller.cpp
  controllers/v1/foldercontroller.h
  controllers/v1/folderinfocontroller.cpp
  controllers/v1/folderinfocontroller.h
  controllers/v1/librariescontroller.cpp
  controllers/v1/librariescontroller.h
  controllers/v1/pagecontroller.cpp
  controllers/v1/pagecontroller.h
  controllers/v1/synccontroller.cpp
  controllers/v1/synccontroller.h
  controllers/v1/updatecomiccontroller.cpp
  controllers/v1/updatecomiccontroller.h
  # Current controller
  controllers/v2/comiccontroller_v2.cpp
  controllers/v2/comiccontroller_v2.h
  controllers/v2/comiccontrollerinreadinglist_v2.cpp
  controllers/v2/comiccontrollerinreadinglist_v2.h
  controllers/v2/comicdownloadinfocontroller_v2.cpp
  controllers/v2/comicdownloadinfocontroller_v2.h
  controllers/v2/comicfullinfocontroller_v2.cpp
  controllers/v2/comicfullinfocontroller_v2.h
  controllers/v2/covercontroller_v2.cpp
  controllers/v2/covercontroller_v2.h
  controllers/v2/errorcontroller_v2.cpp
  controllers/v2/errorcontroller_v2.h
  controllers/v2/favoritescontroller_v2.cpp
  controllers/v2/favoritescontroller_v2.h
  controllers/v2/foldercontentcontroller_v2.cpp
  controllers/v2/foldercontentcontroller_v2.h
  controllers/v2/folderinfocontroller_v2.cpp
  controllers/v2/folderinfocontroller_v2.h
  controllers/v2/librariescontroller_v2.cpp
  controllers/v2/librariescontroller_v2.h
  controllers/v2/pagecontroller_v2.cpp
  controllers/v2/pagecontroller_v2.h
  controllers/v2/readingcomicscontroller_v2.cpp
  controllers/v2/readingcomicscontroller_v2.h
  controllers/v2/readinglistcontentcontroller_v2.cpp
  controllers/v2/readinglistcontentcontroller_v2.h
  controllers/v2/readinglistinfocontroller_v2.cpp
  controllers/v2/readinglistinfocontroller_v2.h
  controllers/v2/readinglistscontroller_v2.cpp
  controllers/v2/readinglistscontroller_v2.h
  controllers/v2/searchcontroller_v2.cpp
  controllers/v2/searchcontroller_v2.h
  controllers/v2/synccontroller_v2.cpp
  controllers/v2/synccontroller_v2.h
  controllers/v2/tagcontentcontroller_v2.cpp
  controllers/v2/tagcontentcontroller_v2.h
  controllers/v2/taginfocontroller_v2.cpp
  controllers/v2/taginfocontroller_v2.h
  controllers/v2/tagscontroller_v2.cpp
  controllers/v2/tagscontroller_v2.h
  controllers/v2/updatecomiccontroller_v2.cpp
  controllers/v2/updatecomiccontroller_v2.h
  # Switchter and WebUI
  controllers/versioncontroller.cpp
  controllers/versioncontroller.h
  controllers/webui/statuspagecontroller.cpp
  controllers/webui/statuspagecontroller.h
  # Other
  requestmapper.cpp
  requestmapper.h
  static.cpp
  static.h
  yacreader_http_server.cpp
  yacreader_http_server.h
  yacreader_http_session_store.cpp
  yacreader_http_session_store.h
  yacreader_http_session.cpp
  yacreader_http_session.h
  yacreader_server_data_helper.cpp
  yacreader_server_data_helper.h)

target_include_directories(
  server
  PRIVATE controllers controllers/v1 controllers/v2
  PUBLIC .)
target_compile_definitions(
  server
  PUBLIC SERVER_VERSION_NUMBER="2.0" DATADIR="${CMAKE_INSTALL_FULL_DATADIR}"
         BINDIR="${CMAKE_INSTALL_FULL_BINDIR}")
target_link_libraries(server QtWebApp_httpserver QtWebApp_templateengine
                      Qt::Sql comic_backend db_helper)
