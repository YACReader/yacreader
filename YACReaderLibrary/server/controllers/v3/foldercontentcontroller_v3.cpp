#include "foldercontentcontroller_v3.h"

#include "comic_item.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "v3_response.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"
#include "yacreader_server_data_helper.h"

#include <QJsonArray>

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

FolderContentControllerV3::FolderContentControllerV3(int libraryId, FolderId folderId)
    : libraryId(libraryId), folderId(folderId)
{
}

void FolderContentControllerV3::service(HttpRequest & /* request */, HttpResponse &response)
{
    const auto libraries = DBHelper::getLibraries();
    const auto libraryUuid = libraries.getLibraryIdFromLegacyId(libraryId);

    bool found = false;
    std::optional<QList<Folder>> folders;
    QList<ComicItem> comics;
    QString connectionName;
    {
        auto db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(libraries.getPath(libraryId)));
        found = DBHelper::loadFolder(folderId.value(), db).has_value();
        if (found) {
            folders = DBHelper::loadSubfolders(folderId, db);
            comics = ComicItems::loadFolder(folderId, db);
        }
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (!found)
        return V3Response::notFound(response);
    if (!folders)
        return V3Response::serverError(response);

    QJsonArray items;
    for (const auto &folder : std::as_const(*folders))
        items.append(YACReaderServerDataHelper::folderToJSON(libraryId, libraryUuid, folder));
    for (const auto &comic : std::as_const(comics))
        items.append(YACReaderServerDataHelper::comicItemToJSON(libraryId, libraryUuid, comic));
    V3Response::json(response, items);
}
