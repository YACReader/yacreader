#include "listcontentcontroller_v3.h"

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

ListContentControllerV3::ListContentControllerV3(int libraryId, ListKind listKind, qulonglong listId)
    : libraryId(libraryId), listKind(listKind), listId(listId)
{
}

void ListContentControllerV3::service(HttpRequest & /* request */, HttpResponse &response)
{
    const auto libraries = DBHelper::getLibraries();
    const auto libraryUuid = libraries.getLibraryIdFromLegacyId(libraryId);

    bool found = false;
    QList<ComicItem> entries;
    QString connectionName;
    {
        auto db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(libraries.getPath(libraryId)));
        switch (listKind) {
        case ListKind::ReadingList:
            found = DBHelper::readingListExists(ReadingListId(listId), db);
            if (found)
                entries = ComicItems::loadReadingList(ReadingListId(listId), db);
            break;
        case ListKind::Label:
            found = DBHelper::labelExists(LabelId(listId), db);
            if (found)
                entries = ComicItems::loadLabel(LabelId(listId), db);
            break;
        }
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (!found)
        return V3Response::notFound(response);

    QJsonArray items;
    for (const auto &entry : std::as_const(entries))
        items.append(YACReaderServerDataHelper::comicItemToJSON(libraryId, libraryUuid, entry));
    V3Response::json(response, items);
}
