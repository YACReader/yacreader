#ifndef LIBRARY_COMIC_MATCH_H
#define LIBRARY_COMIC_MATCH_H

#include "library_ids.h"

#include <QSet>
#include <QStringList>

#include <optional>
#include <variant>

class QSqlDatabase;

// the cover files of the matches of one transaction: the written ones are removed when it fails, the others after
// the commit
struct ComicMatchFiles {
    QStringList writtenCoverPaths;
    QStringList placeholderCoverKeys;
    QStringList comicCoverHashes;
};

// what a missing entry refers to
using MissingComic = std::variant<PlaceholderId, ComicInfoId>;

// Gives the comic_info of a comic with a file to every entry of the placeholder, in every list, and removes its
// placements in the folders that have a file of the comic; the placeholder is deleted when no placement is left.
// A label holds a comic once, so an entry of a label that already has the comic is removed. The comic
// keeps its values and fills its empty ones from the placeholder; it takes the placeholder's cover only when it has
// none. No transaction: the caller runs it in its own. False when the placeholder or the comic is gone, or a write
// fails.
bool matchPlaceholderToComic(PlaceholderId placeholderId, ComicInfoId comicInfoId, const QString &libraryDataPath, QSqlDatabase &db, ComicMatchFiles &files);
// Gives the comic_info of a comic with a file to every entry of a deleted comic, in every list, and deletes the
// deleted comic_info. Labels and the metadata merge are as for a placeholder; read state and rating come from the
// deleted comic when the comic has none, and the later last opened time stays; the comic keeps its current page and
// its cover. No transaction: the caller runs it in its own.
bool matchDeletedComicToComic(ComicInfoId deletedComicInfoId, ComicInfoId comicInfoId, QSqlDatabase &db, ComicMatchFiles &files);
// after the commit, or after the rollback when `committed` is false
void finishComicMatchFiles(const QString &libraryDataPath, const ComicMatchFiles &files, bool committed);

// no transaction; when the label of the entry already has the comic, the entry is removed
bool giveComicToEntry(ListItemId entryId, ComicInfoId comicInfoId, QSqlDatabase &db);

// the names of the reading lists and labels with entries of any of `missing`, except the lists `listIds` of `listKind`
std::optional<QStringList> otherListsWithEntriesOf(const QList<MissingComic> &missing, ListKind listKind, const QSet<qulonglong> &listIds, QSqlDatabase &db);

#endif
