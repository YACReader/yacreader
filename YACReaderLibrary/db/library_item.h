#ifndef __LIBRARY_ITEM_H
#define __LIBRARY_ITEM_H

class LibraryItem
{
public:
	virtual bool isDir() = 0;
	QString name;
	QString path;
	qulonglong parentId;
	qulonglong id;
};

#endif