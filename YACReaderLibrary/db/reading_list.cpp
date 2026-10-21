#include "reading_list.h"

#include "table_projection.h"

#include <QSqlQuery>
#include <QSqlRecord>

const QStringList &ReadingList::columns()
{
    static const QStringList columns = { "parentId", "ordering", "name", "finished", "completed", "folder_id" };
    return columns;
}

namespace {

// `value(k)` is column k of {id} + ReadingList::columns()
template<typename Value>
ReadingList readReadingList(const Value &value)
{
    ReadingList list;
    list.id = ReadingListId(value(0).toULongLong());
    const auto parentId = value(1);
    if (!parentId.isNull()) {
        list.parentId = ReadingListId(parentId.toULongLong());
    }
    list.ordering = value(2).toInt();
    list.name = value(3).toString();
    list.finished = value(4).toBool();
    list.completed = value(5).toBool();
    const auto folderId = value(6);
    if (!folderId.isNull()) {
        list.folderId = folderId.toULongLong();
    }
    return list;
}

// `value(k)` is column k of {id} + Label::columns()
template<typename Value>
Label readLabel(const Value &value)
{
    Label label;
    label.id = LabelId(value(0).toULongLong());
    label.name = value(1).toString();
    label.color = value(2).toString();
    label.ordering = value(3).toInt();
    return label;
}

}

ReadingList ReadingList::fromRecord(const QSqlRecord &record, const QString &alias)
{
    const auto indexes = columnIndexes(record, alias);
    return readReadingList([&](int column) { return record.value(indexes.at(column)); });
}

QList<int> ReadingList::columnIndexes(const QSqlRecord &record, const QString &alias)
{
    return TableProjection::columnIndexes(record, alias, columns());
}

ReadingList ReadingList::fromQuery(const QSqlQuery &query, const QList<int> &columnIndexes)
{
    return readReadingList([&](int column) { return query.value(columnIndexes.at(column)); });
}

void ReadingList::bindTo(QSqlQuery &query) const
{
    query.bindValue(":parentId", parentId ? QVariant(parentId->value()) : QVariant());
    query.bindValue(":ordering", ordering);
    query.bindValue(":name", name);
    query.bindValue(":finished", finished);
    query.bindValue(":completed", completed);
    query.bindValue(":folder_id", folderId ? QVariant(*folderId) : QVariant());
}

const QStringList &Label::columns()
{
    static const QStringList columns = { "name", "color", "ordering" };
    return columns;
}

Label Label::fromRecord(const QSqlRecord &record, const QString &alias)
{
    const auto indexes = columnIndexes(record, alias);
    return readLabel([&](int column) { return record.value(indexes.at(column)); });
}

QList<int> Label::columnIndexes(const QSqlRecord &record, const QString &alias)
{
    return TableProjection::columnIndexes(record, alias, columns());
}

Label Label::fromQuery(const QSqlQuery &query, const QList<int> &columnIndexes)
{
    return readLabel([&](int column) { return query.value(columnIndexes.at(column)); });
}

void Label::bindTo(QSqlQuery &query) const
{
    query.bindValue(":name", name);
    query.bindValue(":color", color);
    query.bindValue(":ordering", ordering);
}
