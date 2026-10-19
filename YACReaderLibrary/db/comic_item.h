#ifndef COMIC_ITEM_H
#define COMIC_ITEM_H

#include "comic_db.h"

#include <QList>

#include <variant>

enum class ListKind {
    ReadingList,
    Label,
};

// `Comic` is also the class that reads comic files, so the cases live in their own namespace.
namespace ComicItemCase {

struct Comic {
    ComicDB comic;
};

struct ComicInList {
    ComicDB comic;
    ListKind listKind;
    qulonglong listId;
};

}

using ComicItem = std::variant<ComicItemCase::Comic, ComicItemCase::ComicInList>;

#endif
