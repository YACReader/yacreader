#include "requestmapper.h"

#include "QsLog.h"
#include "controllers/v2/comiccontroller_v2.h"
#include "controllers/v2/comiccontrollerinreadinglist_v2.h"
#include "controllers/v2/comicdownloadinfocontroller_v2.h"
#include "controllers/v2/comicfullinfocontroller_v2.h"
#include "controllers/v2/covercontroller_v2.h"
#include "controllers/v2/favoritescontroller_v2.h"
#include "controllers/v2/foldercontentcontroller_v2.h"
#include "controllers/v2/folderinfocontroller_v2.h"
#include "controllers/v2/foldermetadatacontroller_v2.h"
#include "controllers/v2/itemupdatecontroller_v2.h"
#include "controllers/v2/librariescontroller_v2.h"
#include "controllers/v2/pagecontroller_v2.h"
#include "controllers/v2/readingcomicscontroller_v2.h"
#include "controllers/v2/readinglistcontentcontroller_v2.h"
#include "controllers/v2/readinglistinfocontroller_v2.h"
#include "controllers/v2/readinglistscontroller_v2.h"
#include "controllers/v2/searchcontroller_v2.h"
#include "controllers/v2/synccontroller_v2.h"
#include "controllers/v2/tagcontentcontroller_v2.h"
#include "controllers/v2/taginfocontroller_v2.h"
#include "controllers/v2/tagscontroller_v2.h"
#include "controllers/v2/updatecomiccontroller_v2.h"
#include "controllers/v2/updatelibrariescontroller_v2.h"
#include "controllers/v3/covercontroller_v3.h"
#include "controllers/v3/foldercontentcontroller_v3.h"
#include "controllers/v3/folderinfocontroller_v3.h"
#include "controllers/v3/listcontentcontroller_v3.h"
#include "controllers/v3/listitemcontroller_v3.h"
#include "controllers/v3/listsectioncontroller_v3.h"
#include "controllers/v3/recentcontroller_v3.h"
#include "controllers/v3/tagscontroller_v3.h"
#include "controllers/v3/v3_response.h"
#include "controllers/versioncontroller.h"
#include "controllers/webui/webuicontroller.h"
#include "db_helper.h"
#include "static.h"
#include "staticfilecontroller.h"
#include "yacreader_http_session.h"
#include "yacreader_libraries.h"

#include <QRegExp>

#include <memory>
#include <optional>

using stefanfrings::HttpRequest;
using stefanfrings::HttpRequestHandler;
using stefanfrings::HttpResponse;

QMutex RequestMapper::mutex;

RequestMapper::RequestMapper(QObject *parent)
    : HttpRequestHandler(parent) { }

void RequestMapper::loadSession(HttpRequest &request, HttpResponse & /* response */)
{
    QMutexLocker locker(&mutex);

    QByteArray token = request.getHeader("x-request-id");

    if (token.isEmpty()) {
        return;
    }

    auto yRecoveredSession = Static::yacreaderSessionStore->getYACReaderSessionHttpSession(token);

    if (yRecoveredSession == nullptr) // session is already alive check if it is needed to update comics
    {
        auto ySession = new YACReaderHttpSession();

        Static::yacreaderSessionStore->addYACReaderHttpSession(token, ySession);
    }
}

void RequestMapper::service(HttpRequest &request, HttpResponse &response)
{
    QByteArray path = request.getPath();

    QLOG_TRACE() << "RequestMapper: path=" << path.data();
    QLOG_TRACE() << "X-Request-Id: " << request.getHeader("x-request-id");

    if (path.startsWith("/v2")) {
        serviceV2(request, response);
    } else if (path.startsWith("/v3")) {
        serviceV3(request, response);
    } else if (path == "/" || path.startsWith("/webui")) {
        serviceWebUI(request, response);
    } else {
        Static::staticFileController->service(request, response);
    }
}

void RequestMapper::serviceWebUI(HttpRequest &request, HttpResponse &response)
{
    WebUIController().service(request, response);
}

void RequestMapper::serviceV2(HttpRequest &request, HttpResponse &response)
{
    QByteArray path = request.getPath();

    QRegExp folderInfo("/v2/library/.+/folder/[0-9]+/info/?"); // get folder info (all comics in a folder including subfolders recursively)
    QRegExp comicDownloadInfo("/v2/library/.+/comic/[0-9]+/info/?"); // get comic info (full download info)
    QRegExp comicOpenForDownloading("/v2/library/.+/comic/[0-9]+/?"); // get comic info (full info + opening)
    QRegExp comicOpenForRemoteReading("/v2/library/.+/comic/[0-9]+/remote/?"); // the server will open for reading the comic
    QRegExp comicOpenForRemoteReadingInAReadingList("/v2/library/.+/reading_list/[0-9]+/comic/[0-9]+/remote/?"); // the server will open for reading the comic
    QRegExp comicFullInfo("/v2/library/.+/comic/[0-9]+/fullinfo/?"); // get comic info
    QRegExp comicUpdate("/v2/library/.+/comic/[0-9]+/update/?"); // get comic info
    QRegExp cover("/v2/library/.+/cover/.+"); // get comic cover (navigation)
    QRegExp comicPage("/v2/library/.+/comic/[0-9]+/page/[0-9]+/?"); // get comic page
    QRegExp comicPageRemote("/v2/library/.+/comic/[0-9]+/page/[0-9]+/remote?"); // get comic page (remote reading)
    QRegExp serverVersion("/v2/version/?");
    QRegExp folderContent("/v2/library/.+/folder/[0-9]+/content/?");
    QRegExp folderMetadata("/v2/library/.+/folder/[0-9]+/metadata/?"); // get the folder metadata json (9.14)
    QRegExp favs("/v2/library/.+/favs/?");
    QRegExp reading("/v2/library/.+/reading/?");
    QRegExp tags("/v2/library/.+/tags/?");
    QRegExp tagContent("/v2/library/.+/tag/[0-9]+/content/?");
    QRegExp tagInfo("/v2/library/.+/tag/[0-9]+/info/?");
    QRegExp readingLists("/v2/library/.+/reading_lists/?");
    QRegExp readingListContent("/v2/library/.+/reading_list/[0-9]+/content/?");
    QRegExp readingListInfo("/v2/library/.+/reading_list/[0-9]+/info/?");
    QRegExp search("/v2/library/.+/search/?");
    QRegExp itemUpdate("/v2/library/[0-9]+/(comic|folder)/[0-9]+/?"); // PATCH: change fields of a comic or a folder

    QRegExp sync("/v2/sync");

    QRegExp librariesUpdate("/v2/libraries/update/?"); // trigger an update of all libraries
    QRegExp librariesUpdateStatus("/v2/libraries/update/status/?"); // poll whether an update is running
    QRegExp librariesUpdateCancel("/v2/libraries/update/cancel/?"); // cancel a running update
    QRegExp libraryUpdate("/v2/library/[0-9]+/update/?"); // trigger an update of a single library
    QRegExp libraryXmlRescan("/v2/library/[0-9]+/rescan-xml/?"); // rescan ComicInfo.xml metadata for a single library

    QRegExp library("/v2/library/([0-9]+)/.+"); // permite verificar que la biblioteca solicitada existe

    path = QUrl::fromPercentEncoding(path).toUtf8();

    if (!sync.exactMatch(path)) // no session is needed for syncback info, until security will be added
        loadSession(request, response);

    // primera petición, se ha hecho un post, se sirven las bibliotecas si la seguridad mediante login no está habilitada
    if (path == "/v2/libraries") // Don't send data to the server using '/' !!!!
    {
        LibrariesControllerV2().service(request, response);
    } else {
        if (serverVersion.exactMatch(path)) {
            VersionController(SERVER_VERSION_NUMBER).service(request, response);
        } else if (sync.exactMatch(path)) {
            SyncControllerV2 syncController;
            syncController.service(request, response);

            for (const auto &libraryId : std::as_const(syncController.changedLibraries)) {
                emit libraryContentChanged(libraryId);
            }
        } else if (librariesUpdate.exactMatch(path) || librariesUpdateStatus.exactMatch(path) || librariesUpdateCancel.exactMatch(path)) {
            UpdateLibrariesControllerV2().service(request, response);
        } else {
            if (library.indexIn(path) != -1 && DBHelper::getLibraries().contains(library.cap(1).toInt())) {
                if (request.getMethod() == "PATCH") {
                    // only the update endpoints accept PATCH, the other routes do not check the method
                    if (itemUpdate.exactMatch(path)) {
                        ItemUpdateControllerV2 itemUpdateController;
                        itemUpdateController.service(request, response);

                        if (!itemUpdateController.changedLibraryId.isNull()) {
                            emit libraryContentChanged(itemUpdateController.changedLibraryId);
                        }
                    } else {
                        response.setStatus(405, "Method Not Allowed");
                        response.write("405 method not allowed", true);
                    }
                } else if (folderInfo.exactMatch(path)) {
                    FolderInfoControllerV2().service(request, response);
                } else if (cover.exactMatch(path)) {
                    CoverControllerV2().service(request, response);
                } else if (comicDownloadInfo.exactMatch(path)) {
                    ComicDownloadInfoControllerV2().service(request, response);
                } else if (comicOpenForRemoteReadingInAReadingList.exactMatch(path)) {
                    ComicControllerInReadingListV2().service(request, response);
                } else if (comicOpenForDownloading.exactMatch(path) || comicOpenForRemoteReading.exactMatch(path)) { // start download or start remote reading
                    ComicControllerV2().service(request, response);
                } else if (comicFullInfo.exactMatch(path)) {
                    ComicFullinfoController_v2().service(request, response);
                } else if (comicPage.exactMatch(path) || comicPageRemote.exactMatch(path)) {
                    PageControllerV2().service(request, response);
                } else if (comicUpdate.exactMatch(path)) {
                    auto updateController = UpdateComicControllerV2();
                    updateController.service(request, response);

                    if (!updateController.error) {
                        emit comicUpdated(updateController.updatedLibraryId, updateController.updatedComicId);
                    }
                } else if (libraryUpdate.exactMatch(path) || libraryXmlRescan.exactMatch(path)) {
                    UpdateLibrariesControllerV2().service(request, response);
                } else if (folderContent.exactMatch(path)) {
                    FolderContentControllerV2().service(request, response);
                } else if (tags.exactMatch(path)) {
                    TagsControllerV2().service(request, response);
                } else if (tagContent.exactMatch(path)) {
                    TagContentControllerV2().service(request, response);
                } else if (favs.exactMatch(path)) {
                    FavoritesControllerV2().service(request, response);
                } else if (reading.exactMatch(path)) {
                    ReadingComicsControllerV2().service(request, response);
                } else if (readingLists.exactMatch(path)) {
                    ReadingListsControllerV2().service(request, response);
                } else if (readingListContent.exactMatch(path)) {
                    ReadingListContentControllerV2().service(request, response);
                } else if (readingListInfo.exactMatch(path)) {
                    ReadingListInfoControllerV2().service(request, response);
                } else if (tagInfo.exactMatch(path)) {
                    TagInfoControllerV2().service(request, response);
                } else if (search.exactMatch(path)) {
                    SearchController().service(request, response);
                } else if (folderMetadata.exactMatch(path)) {
                    FolderMetadataControllerV2().service(request, response);
                } else {
                    response.setStatus(404, "not found");
                    response.write("404 not found", true);
                }
            } else {
                // response.writeText(library.cap(1));
                Static::staticFileController->service(request, response);
            }
        }
    }
}

// v3 has only the routes whose content changed; a v3 client uses v2 for the rest, with the same session
void RequestMapper::serviceV3(HttpRequest &request, HttpResponse &response)
{
    const QByteArray path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();

    QRegExp serverVersion("/v3/version/?");
    QRegExp folderContent("/v3/library/([0-9]+)/folder/([0-9]+)/content/?");
    QRegExp folderInfo("/v3/library/([0-9]+)/folder/([0-9]+)/info/?");
    QRegExp readingListContent("/v3/library/([0-9]+)/reading_list/([0-9]+)/content/?");
    QRegExp labelContent("/v3/library/([0-9]+)/tag/([0-9]+)/content/?");
    QRegExp readingLists("/v3/library/([0-9]+)/reading_lists/?");
    QRegExp readingListFolderContent("/v3/library/([0-9]+)/reading_list_folder/([0-9]+)/content/?");
    QRegExp smartLists("/v3/library/([0-9]+)/smart_lists/?");
    QRegExp smartListFolderContent("/v3/library/([0-9]+)/smart_list_folder/([0-9]+)/content/?");
    QRegExp smartListContent("/v3/library/([0-9]+)/smart_list/([0-9]+)/content/?");
    QRegExp cover("/v3/library/([0-9]+)/cover/(.+)");
    QRegExp tags("/v3/library/([0-9]+)/tags/?");
    QRegExp recent("/v3/library/([0-9]+)/recent/?");
    QRegExp readingList("/v3/library/([0-9]+)/reading_list/([0-9]+)/?");
    QRegExp readingListFolder("/v3/library/([0-9]+)/reading_list_folder/([0-9]+)/?");
    QRegExp smartList("/v3/library/([0-9]+)/smart_list/([0-9]+)/?");
    QRegExp smartListFolder("/v3/library/([0-9]+)/smart_list_folder/([0-9]+)/?");

    loadSession(request, response);

    std::unique_ptr<HttpRequestHandler> controller;
    std::optional<int> libraryId;
    if (serverVersion.exactMatch(path)) {
        controller = std::make_unique<VersionController>(SERVER_V3_VERSION_NUMBER);
    } else if (folderContent.exactMatch(path)) {
        libraryId = folderContent.cap(1).toInt();
        controller = std::make_unique<FolderContentControllerV3>(*libraryId, FolderId(folderContent.cap(2).toULongLong()));
    } else if (folderInfo.exactMatch(path)) {
        libraryId = folderInfo.cap(1).toInt();
        controller = std::make_unique<FolderInfoControllerV3>(*libraryId, FolderId(folderInfo.cap(2).toULongLong()));
    } else if (readingListContent.exactMatch(path)) {
        libraryId = readingListContent.cap(1).toInt();
        controller = std::make_unique<ListContentControllerV3>(*libraryId, ReadingListId(readingListContent.cap(2).toULongLong()));
    } else if (labelContent.exactMatch(path)) {
        libraryId = labelContent.cap(1).toInt();
        controller = std::make_unique<ListContentControllerV3>(*libraryId, LabelId(labelContent.cap(2).toULongLong()));
    } else if (readingLists.exactMatch(path)) {
        libraryId = readingLists.cap(1).toInt();
        controller = std::make_unique<ListSectionControllerV3>(*libraryId, ListFolderKind::ReadingList, std::nullopt);
    } else if (readingListFolderContent.exactMatch(path)) {
        libraryId = readingListFolderContent.cap(1).toInt();
        controller = std::make_unique<ListSectionControllerV3>(*libraryId, ListFolderKind::ReadingList, readingListFolderContent.cap(2).toULongLong());
    } else if (smartLists.exactMatch(path)) {
        libraryId = smartLists.cap(1).toInt();
        controller = std::make_unique<ListSectionControllerV3>(*libraryId, ListFolderKind::SmartList, std::nullopt);
    } else if (smartListFolderContent.exactMatch(path)) {
        libraryId = smartListFolderContent.cap(1).toInt();
        controller = std::make_unique<ListSectionControllerV3>(*libraryId, ListFolderKind::SmartList, smartListFolderContent.cap(2).toULongLong());
    } else if (smartListContent.exactMatch(path)) {
        libraryId = smartListContent.cap(1).toInt();
        controller = std::make_unique<ListContentControllerV3>(*libraryId, SmartListId(smartListContent.cap(2).toULongLong()));
    } else if (cover.exactMatch(path)) {
        libraryId = cover.cap(1).toInt();
        controller = std::make_unique<CoverControllerV3>(*libraryId, cover.cap(2));
    } else if (tags.exactMatch(path)) {
        libraryId = tags.cap(1).toInt();
        controller = std::make_unique<TagsControllerV3>(*libraryId);
    } else if (recent.exactMatch(path)) {
        libraryId = recent.cap(1).toInt();
        controller = std::make_unique<RecentControllerV3>(*libraryId);
    } else if (readingList.exactMatch(path)) {
        libraryId = readingList.cap(1).toInt();
        controller = std::make_unique<ListItemControllerV3>(*libraryId, ReadingListId(readingList.cap(2).toULongLong()));
    } else if (readingListFolder.exactMatch(path)) {
        libraryId = readingListFolder.cap(1).toInt();
        controller = std::make_unique<ListItemControllerV3>(*libraryId, ListFolderId { ListFolderKind::ReadingList, readingListFolder.cap(2).toULongLong() });
    } else if (smartList.exactMatch(path)) {
        libraryId = smartList.cap(1).toInt();
        controller = std::make_unique<ListItemControllerV3>(*libraryId, SmartListId(smartList.cap(2).toULongLong()));
    } else if (smartListFolder.exactMatch(path)) {
        libraryId = smartListFolder.cap(1).toInt();
        controller = std::make_unique<ListItemControllerV3>(*libraryId, ListFolderId { ListFolderKind::SmartList, smartListFolder.cap(2).toULongLong() });
    }

    if (controller == nullptr || (libraryId && !DBHelper::getLibraries().contains(*libraryId)))
        V3Response::notFound(response);
    else if (request.getMethod() != "GET")
        V3Response::methodNotAllowed(response);
    else
        controller->service(request, response);
}
