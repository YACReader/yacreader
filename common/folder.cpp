#include "folder.h"

#include "table_projection.h"

#include <QSqlQuery>
#include <QSqlRecord>

const QStringList &Folder::columns()
{
    static const QStringList columns = { "parentId", "name", "path", "finished", "completed", "numChildren",
                                         "firstChildHash", "customImage", "type", "added", "updated" };
    return columns;
}

namespace {

// `value(k)` is column k of {id} + Folder::columns()
template<typename Value>
Folder readFolder(const Value &value)
{
    Folder folder;
    folder.id = FolderId(value(0).toULongLong());
    folder.parentId = FolderId(value(1).toULongLong());
    folder.name = value(2).toString();
    folder.path = value(3).toString();
    folder.finished = value(4).toBool();
    folder.completed = value(5).toBool();
    folder.numChildren = value(6).toInt();
    folder.firstChildHash = value(7).toString();
    folder.customImage = value(8).toString();
    folder.type = value(9).template value<YACReader::FileType>();
    folder.added = value(10).toLongLong();
    folder.updated = value(11).toLongLong();
    return folder;
}

}

Folder Folder::fromRecord(const QSqlRecord &record, const QString &alias)
{
    const auto indexes = columnIndexes(record, alias);
    return readFolder([&](int column) { return record.value(indexes.at(column)); });
}

QList<int> Folder::columnIndexes(const QSqlRecord &record, const QString &alias)
{
    return TableProjection::columnIndexes(record, alias, columns());
}

Folder Folder::fromQuery(const QSqlQuery &query, const QList<int> &columnIndexes)
{
    return readFolder([&](int column) { return query.value(columnIndexes.at(column)); });
}

void Folder::bindTo(QSqlQuery &query) const
{
    query.bindValue(":parentId", parentId.value());
    query.bindValue(":name", name);
    query.bindValue(":path", path);
    query.bindValue(":finished", finished);
    query.bindValue(":completed", completed);
    query.bindValue(":numChildren", numChildren);
    query.bindValue(":firstChildHash", firstChildHash);
    query.bindValue(":customImage", customImage);
    query.bindValue(":type", static_cast<int>(type));
    query.bindValue(":added", added);
    query.bindValue(":updated", updated);
}
