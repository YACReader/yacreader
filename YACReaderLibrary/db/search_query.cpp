
#include "search_query.h"

#include "comic_db.h"
#include "folder.h"
#include "query_parser.h"
#include "table_projection.h"

#include <QSqlQuery>
#include <QtCore>

#include <optional>

QSqlQuery foldersSearchQuery(QSqlDatabase &db, const QString &filter)
{
    QueryParser parser;
    auto result = parser.parse(filter.toStdString());

    std::string queryString = "SELECT DISTINCT " + TableProjection::selectList("f", Folder::columns()).toStdString() + " " SEARCH_FOLDERS_FROM;
    result.buildSqlString(queryString);
    queryString += " AND f.id <> 1 ORDER BY f.parentId,f.name";

    QSqlQuery selectQuery(db);
    selectQuery.prepare(queryString.c_str());
    result.bindValues(selectQuery);

    selectQuery.exec();

    return selectQuery;
}

namespace {

QSqlQuery comicsQuery(QSqlDatabase &db, const QString &filter, std::optional<int> limit)
{
    QueryParser parser;
    auto result = parser.parse(filter.toStdString());

    std::string queryString = "SELECT " + ComicDB::selectList("c", "ci").toStdString() + " " SEARCH_COMICS_FROM;
    result.buildSqlString(queryString);
    // near the order of the search results (numbers by value, text without case), so the limit always keeps the same comics
    queryString += " ORDER BY (ci.series IS NULL OR TRIM(ci.series) = ''), TRIM(ci.series) COLLATE NOCASE, "
                   "(ci.volume IS NOT NULL AND TRIM(ci.volume) <> ''), TRIM(ci.volume) COLLATE NOCASE, "
                   "ci.number IS NULL, CAST(ci.number AS REAL), ci.number COLLATE NOCASE, c.fileName COLLATE NOCASE, c.path";
    if (limit)
        queryString += " LIMIT :limit";

    QSqlQuery selectQuery(db);
    selectQuery.prepare(queryString.c_str());
    if (limit)
        selectQuery.bindValue(":limit", *limit);
    result.bindValues(selectQuery);

    selectQuery.exec();

    return selectQuery;
}

}

QSqlQuery comicsSearchQuery(QSqlDatabase &db, const QString &filter)
{
    return comicsQuery(db, filter, 500); // TODO, load this value from settings
}

QSqlQuery allComicsSearchQuery(QSqlDatabase &db, const QString &filter)
{
    return comicsQuery(db, filter, std::nullopt);
}
