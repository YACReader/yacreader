#include "missingcomiccontroller_v3.h"

#include "data_base_management.h"
#include "db_helper.h"
#include "overloaded.h"
#include "v3_response.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"
#include "yacreader_server_data_helper.h"

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

MissingComicControllerV3::MissingComicControllerV3(int libraryId, MissingComicId itemId)
    : libraryId(libraryId), itemId(itemId)
{
}

void MissingComicControllerV3::service(HttpRequest & /* request */, HttpResponse &response)
{
    const auto libraries = DBHelper::getLibraries();
    const auto libraryUuid = libraries.getLibraryIdFromLegacyId(libraryId);

    std::optional<QJsonObject> item;
    QString connectionName;
    {
        auto db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(libraries.getPath(libraryId)));
        item = std::visit(overloaded {
                                  [&](ComicInfoId id) -> std::optional<QJsonObject> {
                                      const auto info = DBHelper::loadComicInfo(id, db);
                                      if (!info)
                                          return std::nullopt;
                                      return YACReaderServerDataHelper::fullDeletedComicToJSON(libraryId, libraryUuid, *info);
                                  },
                                  [&](PlaceholderId id) -> std::optional<QJsonObject> {
                                      const auto placeholder = DBHelper::loadPlaceholder(id, db);
                                      if (!placeholder)
                                          return std::nullopt;
                                      return YACReaderServerDataHelper::fullPlaceholderToJSON(libraryId, libraryUuid, *placeholder);
                                  },
                          },
                          itemId);
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (!item)
        return V3Response::notFound(response);
    V3Response::json(response, *item);
}
