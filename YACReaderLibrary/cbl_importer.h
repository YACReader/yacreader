#ifndef CBL_IMPORTER_H
#define CBL_IMPORTER_H

#include "cbl_matcher.h"
#include "cbl_reader.h"
#include "library_ids.h"
#include "placeholder.h"

#include <QList>
#include <QString>

#include <optional>

class QSqlDatabase;

// the placeholder of a book that matched nothing: series, number, volume, format, the date (cover date, else a
// year alone as "//yyyy"), and the Comic Vine issue id; the other provider ids stay in the source of its entries
Placeholder placeholderFromCblBook(const CblBook &book);

// the name in the file, or else the file name without its extension
QString cblListName(const CblReadingList &list, const QString &filePath);

// creates a top-level reading list `name` with one entry per book of `list`, in order, in one transaction; `choices[i]`
// is what book i refers to. A book without a choice gets a new placeholder, which the books of this import that are the
// same issue share (the same Comic Vine id, or else the same series, number, volume, and year). An entry of a
// placeholder keeps the source of its book. Nothing is written when any step fails.
std::optional<ReadingListId> importCblReadingList(const QString &name, const CblReadingList &list, const QList<std::optional<CblCandidate>> &choices, QSqlDatabase &db);

#endif
