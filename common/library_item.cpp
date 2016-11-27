
#include "library_item.h"

LibraryItem &LibraryItem::operator=(const LibraryItem &other)
{
    this->name = other.name;
    this->path = other.path;
    this->parentId = other.parentId;
    this->id = other.id;

    return *this;
}
