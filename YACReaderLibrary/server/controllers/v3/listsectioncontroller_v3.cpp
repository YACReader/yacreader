#include "listsectioncontroller_v3.h"

#include "data_base_management.h"
#include "db_helper.h"
#include "list_folder.h"
#include "reading_list.h"
#include "smart_list.h"
#include "v3_response.h"
#include "yacreader_global.h"
#include "yacreader_libraries.h"
#include "yacreader_server_data_helper.h"

#include <QJsonArray>

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

ListSectionControllerV3::ListSectionControllerV3(int libraryId, ListFolderKind kind, std::optional<qulonglong> folderId)
    : libraryId(libraryId), kind(kind), folderId(folderId)
{
}

void ListSectionControllerV3::service(HttpRequest & /* request */, HttpResponse &response)
{
    const auto libraries = DBHelper::getLibraries();
    const auto libraryUuid = libraries.getLibraryIdFromLegacyId(libraryId);

    bool found = false;
    std::optional<QList<ListFolder>> folders;
    std::optional<QList<ReadingList>> readingLists;
    std::optional<QList<SmartList>> smartLists;
    std::optional<QHash<qulonglong, QString>> coverHashes;
    QString connectionName;
    {
        auto db = DataBaseManagement::loadDatabase(YACReader::LibraryPaths::libraryDataPath(libraries.getPath(libraryId)));
        found = !folderId || DBHelper::listFolderExists({ kind, *folderId }, db);
        if (found) {
            folders = DBHelper::loadListFolders(kind, db);
            switch (kind) {
            case ListFolderKind::ReadingList:
                readingLists = DBHelper::loadReadingLists(db);
                if (readingLists) {
                    readingLists->removeIf([this](const ReadingList &list) { return list.parentId || list.folderId != folderId; });
                    QList<ReadingListId> ids;
                    for (const auto &list : std::as_const(*readingLists))
                        ids.append(list.id);
                    coverHashes = DBHelper::readingListCoverHashes(ids, db);
                }
                break;
            case ListFolderKind::SmartList:
                smartLists = DBHelper::loadSmartLists(db);
                if (smartLists)
                    smartLists->removeIf([this](const SmartList &list) { return list.folderId != folderId; });
                break;
            }
        }
        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (!found)
        return V3Response::notFound(response);
    if (!folders || (kind == ListFolderKind::ReadingList ? !readingLists || !coverHashes : !smartLists))
        return V3Response::serverError(response);

    folders->removeIf([this](const ListFolder &folder) { return folder.parentId != folderId; });
    sortListSectionItems(*folders, [](const ListFolder &folder) { return folder.name; });

    QJsonArray items;
    for (const auto &folder : std::as_const(*folders))
        items.append(YACReaderServerDataHelper::listFolderToJSON(libraryId, libraryUuid, folder));
    if (readingLists) {
        sortListSectionItems(*readingLists, [](const ReadingList &list) { return list.name; });
        for (const auto &list : std::as_const(*readingLists))
            items.append(YACReaderServerDataHelper::sectionReadingListToJSON(libraryId, libraryUuid, list, coverHashes->value(list.id.value())));
    }
    if (smartLists) {
        sortListSectionItems(*smartLists, [](const SmartList &list) { return list.name; });
        for (const auto &list : std::as_const(*smartLists))
            items.append(YACReaderServerDataHelper::smartListToJSON(libraryId, libraryUuid, list));
    }
    V3Response::json(response, items);
}
