#include "folder.h"

Folder::Folder()
    : knownParent(false),
      knownId(false),
      numChildren(-1)
{
}

Folder::Folder(qulonglong folderId, qulonglong parentId, const QString &folderName, const QString &folderPath)
    : knownParent(true),
      knownId(true),
      numChildren(-1)
{
    this->id = folderId;
    this->parentId = parentId;
    this->name = folderName;
    this->path = folderPath;
}

Folder::Folder(qulonglong folderId,
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
               qint64 updated)
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
    this->numChildren = numChildren;
    this->firstChildHash = firstChildHash;
    this->customImage = customImage;
    this->type = type;
    this->added = added;
    this->updated = updated;
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
    this->numChildren = other.numChildren;
    this->firstChildHash = other.firstChildHash;
    this->customImage = other.customImage;
    this->type = other.type;
    this->added = other.added;
    this->updated = other.updated;

    return *this;
}

Folder Folder::rootFolder()
{
    auto root = Folder(1, 1, "root", "/");
    root.type = YACReader::FileType::Comic; // TODO: make this configurable by the user so it can set a default type for a library
    return root;
}

Folder::Folder(const QString &folderName, const QString &folderPath)
    : knownParent(false),
      knownId(false),
      numChildren(-1)
{
    this->name = folderName;
    this->path = folderPath;
}
