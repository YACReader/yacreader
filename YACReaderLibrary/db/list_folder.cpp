#include "list_folder.h"

#include "table_projection.h"

#include <QSqlQuery>
#include <QSqlRecord>

const QStringList &ListFolder::columns()
{
    static const QStringList columns = { "parent_id", "name", "ordering" };
    return columns;
}

QList<int> ListFolder::columnIndexes(const QSqlRecord &record, const QString &alias)
{
    return TableProjection::columnIndexes(record, alias, columns());
}

// the indexes are of {id} + columns()
ListFolder ListFolder::fromQuery(const QSqlQuery &query, const QList<int> &columnIndexes, ListFolderKind kind)
{
    ListFolder folder;
    folder.id = { kind, query.value(columnIndexes.at(0)).toULongLong() };
    const auto parentId = query.value(columnIndexes.at(1));
    if (!parentId.isNull())
        folder.parentId = parentId.toULongLong();
    folder.name = query.value(columnIndexes.at(2)).toString();
    folder.ordering = query.value(columnIndexes.at(3)).toInt();
    return folder;
}
