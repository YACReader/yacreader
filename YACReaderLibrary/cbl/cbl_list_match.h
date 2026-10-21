#ifndef CBL_LIST_MATCH_H
#define CBL_LIST_MATCH_H

#include "cbl_matcher.h"
#include "cbl_reader.h"
#include "cbl_writer.h"
#include "comic_record.h"
#include "library_comic_match.h"

#include <QList>

#include <optional>

class QSqlDatabase;

// an entry of a reading list or label in the match view
struct CblListEntry {
    ListItemId id { };
    qulonglong listId = 0;
    // what it refers to when the view opens: a comic (with or without a file) or a placeholder
    CblCandidate current;
    // its CBL book, else the values of what it refers to
    CblBook book;
};

CblListEntry cblListEntry(const ComicRecord &record, const ListEntry &entry);
// an entry of a reading list in an export: its book and its source
CblExportBook cblExportBook(const ComicRecord &record, const ListEntry &entry);
// the values of a comic or missing entry as a book, for matching and the views
CblBook cblBookOf(const DescriptiveComicMetadata &metadata);
CblLibraryMetadata cblLibraryMetadata(const DescriptiveComicMetadata &metadata);

// what Apply does to one entry
struct CblListChange {
    ListItemId entry { };
    // the comic with a file that it gets; nothing makes it missing
    std::optional<ComicInfoId> comic;
};

// In order, each change uses what its entry refers to when it runs: a missing entry or deleted comic is matched (every
// entry of it, in every list), a comic entry only changes itself, and "make missing" gives a comic entry a new
// placeholder from its CBL source, else from the comic's metadata. No transaction: the caller runs it in its own.
// False when an entry is gone or a write fails.
bool applyCblListChanges(const QList<CblListChange> &changes, const QString &libraryDataPath, QSqlDatabase &db, ComicMatchFiles &files);

#endif
