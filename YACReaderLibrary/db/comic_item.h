#ifndef COMIC_ITEM_H
#define COMIC_ITEM_H

#include "comic_db.h"
#include "list_entry.h"
#include "placeholder.h"

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

// an entry of an expected issue that has no comic in the library
struct PlaceholderInList {
    Placeholder placeholder;
    ListEntry entry;
};

// an issue that is expected in a folder and has no comic there
struct PlaceholderInFolder {
    Placeholder placeholder;
    FolderId folderId;
};

}

using ComicItem = std::variant<ComicItemCase::Comic, ComicItemCase::ComicInList, ComicItemCase::DeletedComicInList, ComicItemCase::PlaceholderInList, ComicItemCase::PlaceholderInFolder>;

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
    RemoveFromFolder, // a missing comic of a folder
    MatchToLibraryComic, // a missing entry or a deleted comic gets a comic with a file
};

#endif
