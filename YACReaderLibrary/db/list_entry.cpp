#include "list_entry.h"

#include "table_projection.h"

#include <QSqlQuery>
#include <QSqlRecord>

const QStringList &ListEntry::columns(ListKind listKind)
{
    static const QStringList readingListColumns = { "reading_list_id", "ordering", "last_known_name", "last_known_path", "source", "source_format" };
    static const QStringList labelColumns = { "label_id", "ordering", "last_known_name", "last_known_path" };

    switch (listKind) {
    case ListKind::ReadingList:
        return readingListColumns;
    case ListKind::Label:
        return labelColumns;
    }
    return readingListColumns;
}

QList<int> ListEntry::columnIndexes(const QSqlRecord &record, const QString &alias, ListKind listKind)
{
    return TableProjection::columnIndexes(record, alias, columns(listKind));
}

ListEntry ListEntry::fromQuery(const QSqlQuery &query, const QList<int> &columnIndexes, ListKind listKind)
{
    const auto value = [&](int column) { return query.value(columnIndexes.at(column)); };

    ListEntry entry;
    entry.id = { listKind, value(0).toULongLong() };
    entry.listId = value(1).toULongLong();
    entry.ordering = value(2).toInt();
    const auto name = value(3);
    const auto path = value(4);
    if (!name.isNull() || !path.isNull()) {
        entry.lastKnownFile = LastKnownFile { name.toString(), path.toString() };
    }
    if (listKind == ListKind::ReadingList && !value(5).isNull())
        entry.source = EntrySource { value(5).toString(), value(6).toString() };
    return entry;
}
