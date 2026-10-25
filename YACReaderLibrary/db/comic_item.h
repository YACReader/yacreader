#ifndef COMIC_ITEM_H
#define COMIC_ITEM_H

#include "comic_db.h"
#include "list_entry.h"
#include "placeholder.h"

#include <QList>

#include <optional>
#include <variant>

class QSqlDatabase;
class QSqlQuery;

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

// the descriptive metadata, which every case has
const DescriptiveComicMetadata &rowMetadata(const ComicItem &item);
// the comic of a row with a file
const ComicDB *rowComic(const ComicItem &item);
// the file name of the row, or the last known one of a deleted comic; a placeholder has none
QString rowFileName(const ComicItem &item);

// shared by the desktop views and the server, so both show the same rows in the same order
namespace ComicItems {

QList<ComicItem> fromQuery(QSqlQuery &query);
// the sorting of lists is done in the sql query
QList<ComicItem> fromQuery(QSqlQuery &query, ListKind listKind);
// the comics and placeholders of a folder, in the folder order
QList<ComicItem> loadFolder(FolderId folderId, QSqlDatabase &db);
// the entries of a label, in their order
QList<ComicItem> loadLabel(LabelId labelId, QSqlDatabase &db);
// the entries of a reading list, then the entries of each of its child lists, in their order
QList<ComicItem> loadReadingList(ReadingListId readingListId, QSqlDatabase &db, bool *hasChildLists = nullptr);
// the comics that match the query of a smart list, in the search order; nothing when the list does not exist, no rows
// when its query cannot be parsed
std::optional<QList<ComicItem>> loadSmartList(SmartListId smartListId, QSqlDatabase &db);
// the comics with a file added in the last `days` days, the newest first
QList<ComicItem> loadRecent(qint64 days, QSqlDatabase &db);

bool folderLessThan(const ComicItem &c1, const ComicItem &c2);
// search results and smart lists: each series stays together, in volume and number order
bool searchLessThan(const ComicItem &c1, const ComicItem &c2);
QList<ComicItem> sortedForFolder(QList<ComicItem> items);
// by series, volume, number, file name, and path, in natural order
QList<ComicItem> sortedForSearch(QList<ComicItem> items);

}

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
