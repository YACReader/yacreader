
#include "folder.h"

Folder::Folder(const Folder &folder)
{
    operator=(folder);
}

Folder &Folder::operator =(const Folder &other)
{
    LibraryItem::operator =(other);

    this->knownParent = other.knownParent;
    this->knownId = other.knownId;
    this->finished = other.finished;
    this->completed = other.completed;

    return *this;
}
