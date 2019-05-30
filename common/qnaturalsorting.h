

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

#endif
