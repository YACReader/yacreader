#include "folder.h"

Folder::Folder()
    : knownParent(false),
      knownId(false),
      manga(false),
      numChildren(-1)
{
}

Folder::Folder(qulonglong folderId, qulonglong parentId, const QString &folderName, const QString &folderPath)
    : knownParent(true),
      knownId(true),
      manga(false),
      numChildren(-1)
{
    this->id = folderId;
    this->parentId = parentId;
    this->name = folderName;
    this->path = folderPath;
}

Folder::Folder(qulonglong folderId, qulonglong parentId, const QString &folderName, const QString &folderPath, bool completed, bool finished, bool manga)
    : knownParent(true),
      knownId(true),
      numChildren(-1)
{
    this->id = folderId;
    this->parentId = parentId;
    this->name = folderName;
    this->path = folderPath;
    this->completed = completed;
    this->finished = finished;
    this->manga = manga;
}

Folder::Folder(const Folder &folder)
{
    operator=(folder);
}

Folder &Folder::operator=(const Folder &other)
{
    LibraryItem::operator=(other);

    this->knownParent = other.knownParent;
    this->knownId = other.knownId;
    this->finished = other.finished;
    this->completed = other.completed;
    this->manga = other.manga;
    this->numChildren = other.numChildren;
    this->firstChildHash = other.firstChildHash;
    this->customImage = other.customImage;

    return *this;
}
Folder::Folder(const QString &folderName, const QString &folderPath)
    : knownParent(false),
      knownId(false),
      numChildren(-1)
{
    this->name = folderName;
    this->path = folderPath;
}
