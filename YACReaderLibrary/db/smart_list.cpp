#include "smart_list.h"

#include "table_projection.h"

#include <QSqlQuery>
#include <QSqlRecord>

const QStringList &SmartList::columns()
{
    static const QStringList columns = { "name", "query", "ordering" };
    return columns;
}

QList<int> SmartList::columnIndexes(const QSqlRecord &record, const QString &alias)
{
    return TableProjection::columnIndexes(record, alias, columns());
}

// the indexes are of {id} + columns()
SmartList SmartList::fromQuery(const QSqlQuery &query, const QList<int> &columnIndexes)
{
    SmartList list;
    list.id = SmartListId(query.value(columnIndexes.at(0)).toULongLong());
    list.name = query.value(columnIndexes.at(1)).toString();
    list.query = query.value(columnIndexes.at(2)).toString();
    list.ordering = query.value(columnIndexes.at(3)).toInt();
    return list;
}
