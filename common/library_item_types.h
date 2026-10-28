#ifndef LIBRARY_ITEM_TYPES_H
#define LIBRARY_ITEM_TYPES_H

#include "comic_db.h"
#include "folder.h"

#include <variant>

using DiskItem = std::variant<Folder, ComicDB>;
using FolderContentItem = std::variant<Folder, ComicDB>;

#endif
