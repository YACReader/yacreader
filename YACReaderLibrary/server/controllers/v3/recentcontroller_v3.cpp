#include "recentcontroller_v3.h"

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

RecentControllerV3::RecentControllerV3(int libraryId)
    : libraryId(libraryId)
{
}

void RecentControllerV3::service(HttpRequest &request, HttpResponse &response)
{
    int days = 7;
    if (request.getParameterMap().contains("days")) {
        bool ok = false;
        days = request.getParameter("days").toInt(&ok);
        if (!ok || days <= 0)
            return V3Response::badRequest(response);
    }

    const auto libraries = DBHelper::getLibraries();
    const auto libraryUuid = libraries.getLibraryIdFromLegacyId(libraryId);

    QList<ComicItem> comics;
    QString connectionName;
    {
        auto db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(libraries.getPath(libraryId)));
        comics = ComicItems::loadRecent(days, db);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    QJsonArray items;
    for (const auto &comic : std::as_const(comics))
        items.append(YACReaderServerDataHelper::comicItemToJSON(libraryId, libraryUuid, comic));
    V3Response::json(response, items);
}
