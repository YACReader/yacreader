#include "listcontentcontroller_v3.h"

#include "comic_item.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "overloaded.h"
#include "v3_response.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"
#include "yacreader_server_data_helper.h"

#include <QJsonArray>

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

ListContentControllerV3::ListContentControllerV3(int libraryId, ContentListId listId)
    : libraryId(libraryId), listId(listId)
{
}

void ListContentControllerV3::service(HttpRequest & /* request */, HttpResponse &response)
{
    const auto libraries = DBHelper::getLibraries();
    const auto libraryUuid = libraries.getLibraryIdFromLegacyId(libraryId);

    std::optional<QList<ComicItem>> entries;
    QString connectionName;
    {
        auto db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(libraries.getPath(libraryId)));
        entries = std::visit(overloaded {
                                     [&db](ReadingListId id) -> std::optional<QList<ComicItem>> {
                                         if (!DBHelper::readingListExists(id, db))
                                             return std::nullopt;
                                         return ComicItems::loadReadingList(id, db);
                                     },
                                     [&db](LabelId id) -> std::optional<QList<ComicItem>> {
                                         if (!DBHelper::labelExists(id, db))
                                             return std::nullopt;
                                         return ComicItems::loadLabel(id, db);
                                     },
                                     [&db](SmartListId id) { return ComicItems::loadSmartList(id, db); },
                             },
                             listId);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (!entries)
        return V3Response::notFound(response);

    QJsonArray items;
    for (const auto &entry : std::as_const(*entries))
        items.append(YACReaderServerDataHelper::comicItemToJSON(libraryId, libraryUuid, entry));
    V3Response::json(response, items);
}
