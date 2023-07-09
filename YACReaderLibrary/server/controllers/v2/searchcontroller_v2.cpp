
#include "searchcontroller_v2.h"

#include "data_base_management.h"
#include "db_helper.h"
#include "yacreader_libraries.h"
#include "search_query.h"

#include <QJsonDocument>
#include <QSqlDatabase>
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
    QString connectionName = "";
    {
        QSqlDatabase db = DataBaseManagement::loadDatabase(libraryDBPath);

        // folders
        try {
            auto sqlQuery = foldersSearchQuery(db, query);
            getFolders(libraryId, sqlQuery, results);
        } catch (const std::exception &e) {
        }

        // comics
        try {
            auto sqlQuery = comicsSearchQuery(db, query);
            getComics(libraryId, sqlQuery, results);
        } catch (const std::exception &e) {
        }

        connectionName = db.connectionName();
    }
    QSqlDatabase::removeDatabase(connectionName);

    QJsonDocument output(results);

    response.write(output.toJson(QJsonDocument::Compact));
}

void SearchController::getFolders(int libraryId, QSqlQuery &sqlQuery, QJsonArray &items)
{
    while (sqlQuery.next()) {
        QJsonObject folder;

        folder["type"] = "folder";
        folder["id"] = sqlQuery.value("id").toString();
        folder["library_id"] = QString::number(libraryId);
        folder["folder_name"] = sqlQuery.value("name").toString();
        folder["num_children"] = sqlQuery.value("numChildren").toInt();
        folder["first_comic_hash"] = sqlQuery.value("firstChildHash").toString();
        // 9.13
        folder["finished"] = sqlQuery.value("finished").toBool();
        folder["completed"] = sqlQuery.value("completed").toBool();
        folder["custom_image"] = sqlQuery.value("customImage").toString();
        folder["file_type"] = sqlQuery.value("type").toInt();
        folder["added"] = sqlQuery.value("added").toLongLong();
        folder["updated"] = sqlQuery.value("updated").toLongLong();

        items.append(folder);
    }
}

void SearchController::getComics(int libraryId, QSqlQuery &sqlQuery, QJsonArray &items)
{
    while (sqlQuery.next()) {
        QJsonObject json;

        json["type"] = "comic";
        json["id"] = sqlQuery.value("id").toString();
        json["library_id"] = QString::number(libraryId);
        json["file_name"] = sqlQuery.value("fileName").toString();
        auto hash = sqlQuery.value("hash").toString();
        json["file_size"] = hash.right(hash.length() - 40);
        json["hash"] = hash;
        json["current_page"] = sqlQuery.value("currentPage").toInt();
        json["num_pages"] = sqlQuery.value("numPages").toInt();
        json["read"] = sqlQuery.value("read").toBool();
        json["cover_size_ratio"] = sqlQuery.value("coverSizeRatio").toFloat();
        json["title"] = sqlQuery.value("title").toString();
        json["number"] = sqlQuery.value("number").toInt();
        json["last_time_opened"] = sqlQuery.value("lastTimeOpened").toLongLong();
        auto typeVariant = sqlQuery.value("type");
        auto type = typeVariant.value<YACReader::FileType>();
        json["manga"] = type == YACReader::FileType::Manga; // legacy, kept for compatibility with old clients
        json["file_type"] = typeVariant.toInt(); // 9.13

        items.append(json);
    }
}
