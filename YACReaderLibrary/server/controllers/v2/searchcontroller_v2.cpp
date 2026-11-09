
#include "searchcontroller_v2.h"

#include "comic_db.h"
#include "data_base_management.h"
#include "db_helper.h"
#include "folder.h"
#include "search_query.h"
#include "yacreader_libraries.h"
#include "yacreader_server_data_helper.h"

#include <QJsonDocument>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QUrl>

SearchController::SearchController() { }

void SearchController::service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response)
{
    response.setHeader("Content-Type", "application/json");

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    int libraryId = pathElements.at(3).toInt();

    auto body = request.getBody();
    QJsonDocument json = QJsonDocument::fromJson(body);
    auto query = json["query"].toString();

    response.setStatus(200, "OK");
    serviceSearch(libraryId, query, response);
}

void SearchController::serviceSearch(int libraryId, const QString &query, stefanfrings::HttpResponse &response)
{
    QJsonArray results;

    // TODO replace + "/yacreaderlibrary" concatenations with getDBPath
    QString libraryDBPath = DBHelper::getLibraries().getDBPath(libraryId);
    auto libraryUuid = DBHelper::getLibraries().getLibraryIdFromLegacyId(libraryId);
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(libraryDBPath);

        // folders
        try {
            auto sqlQuery = foldersSearchQuery(db, query);
            getFolders(libraryId, libraryUuid, sqlQuery, results);
        } catch (const std::exception &e) {
        }

        // comics
        try {
            auto sqlQuery = comicsSearchQuery(db, query);
            getComics(libraryId, libraryUuid, sqlQuery, results);
        } catch (const std::exception &e) {
        }

        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    QJsonDocument output(results);

    response.write(output.toJson(QJsonDocument::Compact));
}

// The results use the same JSON as the folder content, so the clients get the same fields (parent_id, path, etc.)
void SearchController::getFolders(int libraryId, const QUuid &libraryUuid, QSqlQuery &sqlQuery, QJsonArray &items)
{
    const auto indexes = Folder::columnIndexes(sqlQuery.record(), "f");
    while (sqlQuery.next()) {
        items.append(YACReaderServerDataHelper::folderToJSON(libraryId, libraryUuid, Folder::fromQuery(sqlQuery, indexes)));
    }
}

void SearchController::getComics(int libraryId, const QUuid &libraryUuid, QSqlQuery &sqlQuery, QJsonArray &items)
{
    const auto indexes = ComicDB::columnIndexes(sqlQuery.record(), "c", "ci");
    while (sqlQuery.next()) {
        const auto comic = ComicDB::fromQuery(sqlQuery, indexes);
        items.append(YACReaderServerDataHelper::comicToJSON(libraryId, libraryUuid, comic));
    }
}
