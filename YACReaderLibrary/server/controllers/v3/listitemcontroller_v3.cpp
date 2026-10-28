#include "listitemcontroller_v3.h"

#include "data_base_management.h"
#include "db_helper.h"
#include "list_folder.h"
#include "overloaded.h"
#include "reading_list.h"
#include "smart_list.h"
#include "v3_response.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"
#include "yacreader_server_data_helper.h"

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

ListItemControllerV3::ListItemControllerV3(int libraryId, SectionItemId itemId)
    : libraryId(libraryId), itemId(itemId)
{
}

void ListItemControllerV3::service(HttpRequest & /* request */, HttpResponse &response)
{
    const auto libraries = DBHelper::getLibraries();
    const auto libraryUuid = libraries.getLibraryIdFromLegacyId(libraryId);

    std::optional<QJsonObject> item;
    QString connectionName;
    {
        auto db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(libraries.getPath(libraryId)));
        item = std::visit(overloaded {
                                  [&](ReadingListId id) -> std::optional<QJsonObject> {
                                      const auto list = DBHelper::loadReadingList(id, db);
                                      if (!list)
                                          return std::nullopt;
                                      const auto hashes = DBHelper::readingListCoverHashes({ id }, db);
                                      return YACReaderServerDataHelper::sectionReadingListToJSON(libraryId, libraryUuid, *list, hashes ? hashes->value(id.value()) : QString());
                                  },
                                  [&](SmartListId id) -> std::optional<QJsonObject> {
                                      const auto list = DBHelper::loadSmartList(id, db);
                                      if (!list)
                                          return std::nullopt;
                                      return YACReaderServerDataHelper::smartListToJSON(libraryId, libraryUuid, *list);
                                  },
                                  [&](const ListFolderId &id) -> std::optional<QJsonObject> {
                                      const auto folder = DBHelper::loadListFolder(id, db);
                                      if (!folder)
                                          return std::nullopt;
                                      return YACReaderServerDataHelper::listFolderToJSON(libraryId, libraryUuid, *folder);
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
