#ifndef __FOLDER_H
#define __FOLDER_H

#include "library_item.h"

#include <QList>

class Folder : public LibraryItem
{
public:
	bool knownParent;
	bool knownId;
	
	Folder():knownParent(false), knownId(false){};
	Folder(qulonglong sid, qulonglong pid,QString fn, QString fp):knownParent(true), knownId(true){id = sid; parentId = pid;name = fn; path = fp;};
	Folder(QString fn, QString fp):knownParent(false), knownId(false){name = fn; path = fp;};
	void setId(qulonglong sid){id = sid;knownId = true;};
	void setFather(qulonglong pid){parentId = pid;knownParent = true;};
	bool isDir(){return true;};
};

#endif