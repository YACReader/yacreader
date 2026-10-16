#ifndef COMIC_ITEM_H
#define COMIC_ITEM_H

#include "comic_db.h"
#include "list_entry.h"

#include <QHashFunctions>
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

// an entry whose comic_info has no file in the library
struct DeletedComicInList {
    ComicInfo info;
    ListEntry entry;
};

}

using ComicItem = std::variant<ComicItemCase::Comic, ComicItemCase::ComicInList, ComicItemCase::DeletedComicInList>;

// what can be done with a row; ComicModel::supports answers it for each case
enum class ComicAction {
    Open,
    OpenContainingFolder,
    ChangeFile, // rename, organize, delete from disk, cover from the file
    SaveCover,
    SetReadState, // read, unread, rating
    SetType,
    EditMetadata,
    DeleteMetadata,
    SearchComicVine,
    AssignNumbers,
    SetCustomCover,
    AddToFavorites,
    AddToList,
    RemoveFromList,
};

// which row this is: a comic in folders and special lists, an entry in reading lists and labels
using ComicRowId = std::variant<ComicId, ListItemId>;

inline size_t qHash(const ComicRowId &key, size_t seed = 0) noexcept
{
    return std::visit([&key, seed](const auto &id) { return qHashMulti(seed, key.index(), id); }, key);
}

#endif
