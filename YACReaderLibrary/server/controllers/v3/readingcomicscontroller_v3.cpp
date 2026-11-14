#include "readingcomicscontroller_v3.h"

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

ReadingComicsControllerV3::ReadingComicsControllerV3(int libraryId)
    : libraryId(libraryId)
{
}

void ReadingComicsControllerV3::service(HttpRequest &request, HttpResponse &response)
{
    Q_UNUSED(request);

    const auto libraries = DBHelper::getLibraries();
    const auto libraryUuid = libraries.getLibraryIdFromLegacyId(libraryId);

    QList<ComicItem> comics;
    QString connectionName;
    {
        auto db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(libraries.getPath(libraryId)));
        comics = ComicItems::loadReading(db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    QJsonArray items;
    for (const auto &comic : std::as_const(comics))
        items.append(YACReaderServerDataHelper::comicItemToJSON(libraryId, libraryUuid, comic));
    V3Response::json(response, items);
}
