#ifndef LIST_FOLDER_H
#define LIST_FOLDER_H

#include "library_ids.h"

#include <QString>
#include <QStringList>

#include <optional>

class QSqlQuery;
class QSqlRecord;

// a folder of the reading lists or of the smart lists section; it holds lists of its kind and folders of its kind
struct ListFolder {
    ListFolderId id { };
    // a folder of the same kind; nothing at the top level
    std::optional<qulonglong> parentId;
    QString name;
    int ordering = 0;

    static const QStringList &columns();
    static QList<int> columnIndexes(const QSqlRecord &record, const QString &alias);
    static ListFolder fromQuery(const QSqlQuery &query, const QList<int> &columnIndexes, ListFolderKind kind);
};

#endif
