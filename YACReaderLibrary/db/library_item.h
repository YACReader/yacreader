#ifndef __LIBRARY_ITEM_H
#define __LIBRARY_ITEM_H

#include <QSqlDatabase>

class LibraryItem
{
public:
	virtual bool isDir() = 0;
	virtual void removeFromDB(QSqlDatabase & db) = 0;
	QString name;
	QString path;
	qulonglong parentId;
	qulonglong id;
};

#endif