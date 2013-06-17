

#ifndef __QNATURALSORTING_H
#define __QNATURALSORTING_H

#include <QString>
#include <QFileInfo>

bool naturalSortLessThanCS( const QString &left, const QString &right );
bool naturalSortLessThanCI( const QString &left, const QString &right );
bool naturalSortLessThanCIFileInfo(const QFileInfo & left,const QFileInfo & right);

#endif