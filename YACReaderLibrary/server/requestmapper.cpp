#include "requestmapper.h"
#include "static.h"
#include "staticfilecontroller.h"

#include "controllers/versioncontroller.h"

#include "controllers/v2/librariescontroller_v2.h"
#include "controllers/v2/covercontroller_v2.h"
#include "controllers/v2/comiccontroller_v2.h"
#include "controllers/v2/folderinfocontroller_v2.h"
#include "controllers/v2/pagecontroller_v2.h"
#include "controllers/v2/updatecomiccontroller_v2.h"
#include "controllers/v2/comicdownloadinfocontroller_v2.h"
#include "controllers/v2/synccontroller_v2.h"
#include "controllers/v2/foldercontentcontroller_v2.h"
#include "controllers/v2/tagscontroller_v2.h"
#include "controllers/v2/tagcontentcontroller_v2.h"
#include "controllers/v2/taginfocontroller_v2.h"
#include "controllers/v2/favoritescontroller_v2.h"
#include "controllers/v2/readingcomicscontroller_v2.h"
#include "controllers/v2/readinglistscontroller_v2.h"
#include "controllers/v2/readinglistcontentcontroller_v2.h"
#include "controllers/v2/readinglistinfocontroller_v2.h"
#include "controllers/v2/comicfullinfocontroller_v2.h"
#include "controllers/v2/comiccontrollerinreadinglist_v2.h"
#include "controllers/v2/searchcontroller_v2.h"
#include "controllers/v2/foldermetadatacontroller_v2.h"

#include "controllers/webui/statuspagecontroller.h"

#include "db_helper.h"
#include "yacreader_libraries.h"

#include "yacreader_http_session.h"

#include "QsLog.h"

#include <QRegExp>

using stefanfrings::HttpRequest;
using stefanfrings::HttpRequestHandler;
using stefanfrings::HttpResponse;

QMutex RequestMapper::mutex;

RequestMapper::RequestMapper(QObject *parent)
    : HttpRequestHandler(parent) { }

void RequestMapper::loadSessionV2(HttpRequest &request, HttpResponse & /* response */)
{
    QMutexLocker locker(&mutex);

    QByteArray token = request.getHeader("x-request-id");

    if (token.isEmpty()) {
        return;
    }

    auto yRecoveredSession = Static::yacreaderSessionStore->getYACReaderSessionHttpSession(token);

    if (yRecoveredSession == nullptr) // session is already alive check if it is needed to update comics
    {
        auto ySession = new YACReaderHttpSession(this);

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
    } else if (path.startsWith("/webui")) {
        serviceWebUI(request, response);
    } else {
        response.setStatus(404, "not found");
        response.write("404 not found", true);
    }
}

void RequestMapper::serviceWebUI(HttpRequest &request, HttpResponse &response)
{
    StatusPageController().service(request, response);
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
    QRegExp comicClose("/v2/library/.+/comic/[0-9]+/close/?"); // the server will close the comic and free memory
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

    QRegExp sync("/v2/sync");

    QRegExp library("/v2/library/([0-9]+)/.+"); // permite verificar que la biblioteca solicitada existe

    path = QUrl::fromPercentEncoding(path).toUtf8();

    if (!sync.exactMatch(path)) // no session is needed for syncback info, until security will be added
        loadSessionV2(request, response);

    // primera petición, se ha hecho un post, se sirven las bibliotecas si la seguridad mediante login no está habilitada
    if (path == "/v2/libraries") // Don't send data to the server using '/' !!!!
    {
        LibrariesControllerV2().service(request, response);
    } else {
        if (serverVersion.exactMatch(path)) {
            VersionController().service(request, response);
        } else if (sync.exactMatch(path)) {
            SyncControllerV2().service(request, response);
            emit clientSync();
        } else {
            if (library.indexIn(path) != -1 && DBHelper::getLibraries().contains(library.cap(1).toInt())) {
                if (folderInfo.exactMatch(path)) {
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
