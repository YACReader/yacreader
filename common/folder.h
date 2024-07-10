#ifndef __FOLDER_H
#define __FOLDER_H

#include "library_item.h"
#include "yacreader_global.h"

#include <QList>

class Folder : public LibraryItem
{
public:
    bool knownParent;
    bool knownId;

    bool finished; // finished means read, the user has read all the content in this folder
    bool completed; // completed means the folder has all the content, e.g. a series got its final issue and the user has collected all of them

    qint32 numChildren; //-1 for unknown number of children
    QString firstChildHash; // empty for unknown first child
    QString customImage; // empty for none custom image

    YACReader::FileType type;
    qint64 added; // epoch in seconds
    qint64 updated; // epoch in seconds

    Folder();
    Folder(qulonglong folderId, qulonglong parentId, const QString &folderName, const QString &folderPath);
    Folder(qulonglong folderId,
           qulonglong parentId,
           const QString &folderName,
           const QString &folderPath,
           bool completed,
           bool finished,
           int numChildren,
           const QString &firstChildHash,
           const QString &customImage,
           YACReader::FileType type,
           qint64 added,
           qint64 updated);
    Folder(const QString &folderName, const QString &folderPath);
    Folder(const Folder &folder);
    Folder &operator=(const Folder &other);

    inline void setId(qulonglong sid)
    {
        id = sid;
        knownId = true;
    }
    inline void setFather(qulonglong pid)
    {
        parentId = pid;
        knownParent = true;
    }

    inline bool isDir() const
    {
        return true;
    }
};

#endif
