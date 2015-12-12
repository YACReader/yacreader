#include "qnaturalsorting.h"

#include <QCollator>



int naturalCompare(const QString &s1, const QString &s2,  Qt::CaseSensitivity caseSensitivity)
{
    QCollator c;
    c.setCaseSensitivity(caseSensitivity);
    c.setNumericMode(true);
    return c.compare(s1, s2);
}
bool naturalSortLessThanCS( const QString &left, const QString &right )
{
  return (naturalCompare( left, right, Qt::CaseSensitive ) < 0);
}

bool naturalSortLessThanCI( const QString &left, const QString &right )
{
	return (naturalCompare( left, right, Qt::CaseInsensitive ) < 0);
}

bool naturalSortLessThanCIFileInfo(const QFileInfo & left,const QFileInfo & right)
{
	return naturalSortLessThanCI(left.fileName(),right.fileName());
}

bool naturalSortLessThanCILibraryItem(LibraryItem * left, LibraryItem * right)
{
	return naturalSortLessThanCI(left->name,right->name);
}
