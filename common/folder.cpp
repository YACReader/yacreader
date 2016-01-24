#include "folder.h"

Folder::Folder()
    :knownParent(false),
     knownId(false),
     numChildren(-1),
     firstChildId(0)
{}

Folder::Folder(qulonglong folderId, qulonglong parentId, const QString &folderName, const QString &folderPath)
    :knownParent(true),
     knownId(true),
     numChildren(-1),
     firstChildId(0)
{
     this->id = folderId;
     this->parentId = parentId;
     this->name = folderName;
     this->path = folderPath;
}

Folder::Folder(const QString & folderName, const QString & folderPath)
    :knownParent(false),
     knownId(false),
     numChildren(-1),
     firstChildId(0)
{
    this->name = folderName;
    this->path = folderPath;
}
