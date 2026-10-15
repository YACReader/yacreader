#include "reading_list.h"

#include "table_projection.h"

#include <QSqlQuery>
#include <QSqlRecord>

using TableProjection::value;

const QStringList &ReadingList::columns()
{
    static const QStringList columns = { "parentId", "ordering", "name", "finished", "completed" };
    return columns;
}

ReadingList ReadingList::fromRecord(const QSqlRecord &record, const QString &alias)
{
    ReadingList list;
    list.id = ReadingListId(value(record, alias, "id").toULongLong());
    const auto parentId = value(record, alias, "parentId");
    if (!parentId.isNull()) {
        list.parentId = ReadingListId(parentId.toULongLong());
    }
    list.ordering = value(record, alias, "ordering").toInt();
    list.name = value(record, alias, "name").toString();
    list.finished = value(record, alias, "finished").toBool();
    list.completed = value(record, alias, "completed").toBool();
    return list;
}

void ReadingList::bindTo(QSqlQuery &query) const
{
    query.bindValue(":parentId", parentId ? QVariant(parentId->value()) : QVariant());
    query.bindValue(":ordering", ordering);
    query.bindValue(":name", name);
    query.bindValue(":finished", finished);
    query.bindValue(":completed", completed);
}

const QStringList &Label::columns()
{
    static const QStringList columns = { "name", "color", "ordering" };
    return columns;
}

Label Label::fromRecord(const QSqlRecord &record, const QString &alias)
{
    Label label;
    label.id = LabelId(value(record, alias, "id").toULongLong());
    label.name = value(record, alias, "name").toString();
    label.color = value(record, alias, "color").toString();
    label.ordering = value(record, alias, "ordering").toInt();
    return label;
}

void Label::bindTo(QSqlQuery &query) const
{
    query.bindValue(":name", name);
    query.bindValue(":color", color);
    query.bindValue(":ordering", ordering);
}
