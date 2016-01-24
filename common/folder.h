#ifndef __FOLDER_H
#define __FOLDER_H

#include "library_item.h"

#include <QList>

class Folder : public LibraryItem
{
public:
	bool knownParent;
	bool knownId;

    qint32 numChildren; //-1 for unknown number of children
    qulonglong firstChildId; //0 for unknown first child
    QString customImage; //empty for none custom image
	
	Folder():knownParent(false), knownId(false){};
	Folder(qulonglong sid, qulonglong pid,QString fn, QString fp):knownParent(true), knownId(true){id = sid; parentId = pid;name = fn; path = fp;};
	Folder(QString fn, QString fp):knownParent(false), knownId(false){name = fn; path = fp;};
	void setId(qulonglong sid){id = sid;knownId = true;};
	void setFather(qulonglong pid){parentId = pid;knownParent = true;};
    bool isDir() {return true;};
    bool isFinished() const {return finished;};
    bool isCompleted() const {return completed;};
    void setFinished(bool b) {finished = b;};
    void setCompleted(bool b) {completed = b;};

private:
    bool finished;
    bool completed;
};

#endif
