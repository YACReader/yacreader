#ifndef __LIBRARY_ITEM_H
#define __LIBRARY_ITEM_H

#include <QObject>

class LibraryItem : public QObject
{
    Q_OBJECT
public:
    virtual bool isDir() const = 0;
    LibraryItem & operator=(const LibraryItem & other);
	QString name;
	QString path;
	qulonglong parentId;
	qulonglong id;
};

#endif
