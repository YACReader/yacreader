

#ifndef __QNATURALSORTING_H
#define __QNATURALSORTING_H

#include <QString>
#include <QFileInfo>
#include "library_item.h"

int naturalCompare(const QString &s1, const QString &s2, Qt::CaseSensitivity caseSensitivity);
bool naturalSortLessThanCS(const QString &left, const QString &right);
bool naturalSortLessThanCI(const QString &left, const QString &right);
bool naturalSortLessThanCIFileInfo(const QFileInfo &left, const QFileInfo &right);
bool naturalSortLessThanCILibraryItem(LibraryItem *left, LibraryItem *right);

/* TODO, update to use the issue number once the iOS client supports it
 * see DBHelper::getFolderComicsFromLibraryForReading
 * NOTE, use this only in the server side for now, this way of sorting just matchs what's used in the iOS client
 **/
struct LibraryItemSorter {
    bool operator()(const LibraryItem *a, const LibraryItem *b) const
    {
        return naturalSortLessThanCI(a->name, b->name);
    }
};

#endif
