#include "folderinfocontroller_v3.h"

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

namespace {

bool appendFolderTree(FolderId folderId, QSqlDatabase &db, QList<ComicItem> &comics)
{
    comics.append(ComicItems::loadFolder(folderId, db));
    const auto subfolders = DBHelper::loadSubfolders(folderId, db);
    if (!subfolders)
        return false;
    for (const auto &folder : *subfolders) {
        if (!appendFolderTree(folder.id, db, comics))
            return false;
    }
    return true;
}

}

FolderInfoControllerV3::FolderInfoControllerV3(int libraryId, FolderId folderId)
    : libraryId(libraryId), folderId(folderId)
{
}

void FolderInfoControllerV3::service(HttpRequest & /* request */, HttpResponse &response)
{
    const auto libraries = DBHelper::getLibraries();
    const auto libraryUuid = libraries.getLibraryIdFromLegacyId(libraryId);

    bool found = false;
    bool loaded = false;
    QList<ComicItem> comics;
    QString connectionName;
    {
        auto db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(libraries.getPath(libraryId)));
        found = DBHelper::loadFolder(folderId.value(), db).has_value();
        if (found)
            loaded = appendFolderTree(folderId, db, comics);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (!found)
        return V3Response::notFound(response);
    if (!loaded)
        return V3Response::serverError(response);

    QJsonArray items;
    for (const auto &comic : std::as_const(comics))
        items.append(YACReaderServerDataHelper::comicItemToJSON(libraryId, libraryUuid, comic));
    V3Response::json(response, items);
}
