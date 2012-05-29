#ifndef __COMIC_H
#define __COMIC_H

#include "library_item.h"
#include <QSqlDatabase>
#include <QList>

class Comic : public LibraryItem
{
public:
	qulonglong comicInfoId;
	QString hash;

	Comic();
	Comic(qulonglong cparentId, qulonglong ccomicInfoId, QString cname, QString cpath, QString chash);
	//Comic(QString fn, QString fp):name(fn),path(fp),knownParent(false), knownId(false){};
	qulonglong insert(QSqlDatabase & db);
	static QList<LibraryItem *> getComicsFromParent(qulonglong parentId, QSqlDatabase & db);
	bool isDir();
	void removeFromDB(QSqlDatabase & db);
};


#endif