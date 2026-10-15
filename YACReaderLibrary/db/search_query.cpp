
#include "search_query.h"

#include "folder.h"
#include "query_parser.h"
#include "table_projection.h"

#include <QSqlQuery>
#include <QtCore>

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

QSqlQuery comicsSearchQuery(QSqlDatabase &db, const QString &filter)
{
    QueryParser parser;
    auto result = parser.parse(filter.toStdString());

    std::string queryString(SEARCH_COMICS_QUERY);
    result.buildSqlString(queryString);
    queryString += " LIMIT :limit";

    QSqlQuery selectQuery(db);
    selectQuery.prepare(queryString.c_str());
    selectQuery.bindValue(":limit", 500); // TODO, load this value from settings
    result.bindValues(selectQuery);

    selectQuery.exec();

    return selectQuery;
}
