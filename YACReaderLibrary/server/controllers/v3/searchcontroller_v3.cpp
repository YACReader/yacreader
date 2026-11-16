#include "searchcontroller_v3.h"

#include "comic_item.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "folder.h"
#include "search_query.h"
#include "v3_response.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"
#include "yacreader_server_data_helper.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QSqlQuery>
#include <QSqlRecord>

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

SearchControllerV3::SearchControllerV3(int libraryId)
    : libraryId(libraryId)
{
}

void SearchControllerV3::service(HttpRequest &request, HttpResponse &response)
{
    const auto query = QJsonDocument::fromJson(request.getBody())["query"].toString();

    const auto libraries = DBHelper::getLibraries();
    const auto libraryUuid = libraries.getLibraryIdFromLegacyId(libraryId);

    QList<Folder> folders;
    QList<ComicItem> comics;
    QString connectionName;
    {
        auto db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(libraries.getPath(libraryId)));
        try {
            auto sqlQuery = foldersSearchQuery(db, query);
            const auto indexes = Folder::columnIndexes(sqlQuery.record(), "f");
            while (sqlQuery.next())
                folders.append(Folder::fromQuery(sqlQuery, indexes));
        } catch (const std::exception &) {
        }
        try {
            auto sqlQuery = comicsSearchQuery(db, query);
            comics = ComicItems::sortedForSearch(ComicItems::fromQuery(sqlQuery));
        } catch (const std::exception &) {
        }
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    QJsonArray items;
    for (const auto &folder : std::as_const(folders))
        items.append(YACReaderServerDataHelper::folderToJSON(libraryId, libraryUuid, folder));
    for (const auto &comic : std::as_const(comics))
        items.append(YACReaderServerDataHelper::comicItemToJSON(libraryId, libraryUuid, comic));
    V3Response::json(response, items);
}
