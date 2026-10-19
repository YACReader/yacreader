#ifndef LIST_ENTRY_H
#define LIST_ENTRY_H

#include "library_ids.h"

#include <QString>
#include <QStringList>

#include <optional>

class QSqlQuery;
class QSqlRecord;

struct LastKnownFile {
    QString name;
    QString path;

    friend bool operator==(const LastKnownFile &, const LastKnownFile &) = default;
};

// a row of reading_list_entry or label_entry; what the entry refers to is the case of the row that holds it
struct ListEntry {
    ListItemId id { };
    qulonglong listId = 0;
    int ordering = 0;
    std::optional<LastKnownFile> lastKnownFile;

    friend bool operator==(const ListEntry &, const ListEntry &) = default;

    static const QStringList &columns(ListKind listKind);
    static QList<int> columnIndexes(const QSqlRecord &record, const QString &alias, ListKind listKind);
    static ListEntry fromQuery(const QSqlQuery &query, const QList<int> &columnIndexes, ListKind listKind);
};

#endif
