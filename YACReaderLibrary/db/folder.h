#ifndef __FOLDER_H
#define __FOLDER_H

#include "library_item.h"

#include <QSqlDatabase>
#include <QList>

class Folder : public LibraryItem
{
public:
	bool knownParent;
	bool knownId;
	
	Folder():knownParent(false), knownId(false){};
	Folder(qulonglong sid, qulonglong pid,QString fn, QString fp):knownParent(true), knownId(true){id = sid; parentId = pid;name = fn; path = fp;};
	Folder(QString fn, QString fp):knownParent(false), knownId(false){name = fn; path = fp;};
	Folder(qulonglong id, QSqlDatabase & db);//loads a folder from db;
	void setId(qulonglong sid){id = sid;knownId = true;};
	void setFather(qulonglong pid){parentId = pid;knownParent = true;};
	static QList<LibraryItem *> getFoldersFromParent(qulonglong parentId, QSqlDatabase & db);
	qulonglong insert(QSqlDatabase & db);
	bool isDir(){return true;};
	void removeFromDB(QSqlDatabase & db);
};

#endif