#ifndef __FOLDER_H
#define __FOLDER_H

#include "library_item.h"

#include <QList>

class Folder : public LibraryItem
{
public:
    bool knownParent;
    bool knownId;

    Folder();
    Folder(qulonglong folderId, qulonglong parentId, const QString &folderName, const QString &folderPath);
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

    inline bool isFinished() const
    {
        return finished;
    }

    inline bool isCompleted() const
    {
        return completed;
    }

    inline bool isManga() const
    {
        return manga;
    }

    inline void setFinished(bool b)
    {
        finished = b;
    }

    inline void setCompleted(bool b)
    {
        completed = b;
    }

    inline void setManga(bool b)
    {
        manga = b;
    }

    inline qint32 getNumChildren() const
    {
        return numChildren;
    }

    inline void setNumChildren(const qint32 v)
    {
        numChildren = v;
    }

    inline QString getFirstChildHash() const
    {
        return firstChildHash;
    }

    inline void setFirstChildHash(const QString &v)
    {
        firstChildHash = v;
    }

    inline QString getCustomImage() const
    {
        return customImage;
    }

    inline void setCustomImage(const QString &s)
    {
        customImage = s;
    }

private:
    bool finished;
    bool completed;
    bool manga;

    qint32 numChildren; //-1 for unknown number of children
    QString firstChildHash; // empty for unknown first child
    QString customImage; // empty for none custom image
};

#endif
