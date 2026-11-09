#include "foldercontentcontroller_v2.h"

#include "QsLog.h"
#include "comic_db.h"
#include "db_helper.h"
#include "folder.h"
#include "library_item_types.h"
#include "overloaded.h"
#include "qnaturalsorting.h"
#include "yacreader_libraries.h"
#include "yacreader_server_data_helper.h"

#include <QUrl>

#include <chrono>

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;
using namespace std;

namespace {

/* Name-only ordering for server responses that mix folders and comics in a single
 * list, where there is no issue number to lean on. Reading order is not this: for
 * the comics of a folder use DBHelper::getFolderComicsFromLibraryForReading, which
 * sorts by issue number the way the clients do.
 **/
bool folderContentItemLessThan(const FolderContentItem &a, const FolderContentItem &b)
{
    const auto name = [](const FolderContentItem &item) {
        return std::visit(overloaded {
                                  [](const Folder &folder) { return folder.name; },
                                  [](const ComicDB &comic) { return comic.file.fileName; },
                          },
                          item);
    };
    return naturalSortLessThanCI(name(a), name(b));
}

}

FolderContentControllerV2::FolderContentControllerV2() { }

void FolderContentControllerV2::service(HttpRequest &request, HttpResponse &response)
{
    response.setHeader("Content-Type", "application/json");

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    int libraryId = pathElements.at(3).toInt();
    qulonglong parentId = pathElements.at(5).toULongLong();

    serviceContent(libraryId, parentId, response);

    response.setStatus(200, "OK");
    response.write("", true);
}

void FolderContentControllerV2::serviceContent(const int &library, const qulonglong &folderId, HttpResponse &response)
{
#ifdef QT_DEBUG
    auto started = std::chrono::high_resolution_clock::now();
#endif
    auto libraryUuid = DBHelper::getLibraries().getLibraryIdFromLegacyId(library);

    QList<FolderContentItem> folderContent;
    for (const auto &folder : DBHelper::getFolderSubfoldersFromLibrary(library, folderId))
        folderContent.append(folder);
    for (const auto &comic : DBHelper::getFolderComicsFromLibrary(library, folderId))
        folderContent.append(comic);

    std::sort(folderContent.begin(), folderContent.end(), folderContentItemLessThan);

    QJsonArray items;

    for (const auto &item : folderContent) {
        items.append(std::visit(overloaded {
                                        [&](const Folder &folder) { return YACReaderServerDataHelper::folderToJSON(library, libraryUuid, folder); },
                                        [&](const ComicDB &comic) { return YACReaderServerDataHelper::comicToJSON(library, libraryUuid, comic); },
                                },
                                item));
    }

    QJsonDocument output(items);

    response.write(output.toJson(QJsonDocument::Compact));
#ifdef QT_DEBUG
    auto done = std::chrono::high_resolution_clock::now();

    QLOG_TRACE() << "num items = " << items.count();
    QLOG_TRACE() << std::chrono::duration_cast<std::chrono::milliseconds>(done - started).count();
#endif
}
