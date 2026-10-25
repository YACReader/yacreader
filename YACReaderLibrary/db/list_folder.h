#ifndef LIST_FOLDER_H
#define LIST_FOLDER_H

#include "library_ids.h"
#include "qnaturalsorting.h"

#include <QString>
#include <QStringList>

#include <algorithm>
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

// the order of the folders, and of the lists, in a folder of a lists section, from their load order; the insert by
// name puts an item before the ones with an equal name, so the last one loaded comes first
template<typename Item, typename Name>
void sortListSectionItems(QList<Item> &items, Name name)
{
    std::reverse(items.begin(), items.end());
    std::stable_sort(items.begin(), items.end(), [&name](const Item &a, const Item &b) { return naturalSortLessThanCI(name(a), name(b)); });
}

#endif
