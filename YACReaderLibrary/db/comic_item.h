#ifndef COMIC_ITEM_H
#define COMIC_ITEM_H

#include "comic_db.h"
#include "list_entry.h"

#include <QList>

#include <variant>

// `Comic` is also the class that reads comic files, so the cases live in their own namespace.
namespace ComicItemCase {

struct Comic {
    ComicDB comic;
};

struct ComicInList {
    ComicDB comic;
    ListEntry entry;
};

}

using ComicItem = std::variant<ComicItemCase::Comic, ComicItemCase::ComicInList>;

#endif
