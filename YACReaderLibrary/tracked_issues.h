#ifndef TRACKED_ISSUES_H
#define TRACKED_ISSUES_H

#include "descriptive_comic_metadata.h"
#include "library_ids.h"

#include <QStringList>

#include <optional>

class QSqlDatabase;

// the longest series have fewer issues
constexpr int maxTrackedIssues = 3000;

// the issues of an "Issues" text, like the pages of a print dialog: parts separated by commas; a part is a range only
// when both sides are whole numbers ("20-120"), else it is one issue as written ("-1", "1-A", "0.5", "Annual 1")
struct TrackedIssues {
    // in order, each number once; a range has no leading zeros
    QStringList numbers;
    int repeated = 0;
    // false for a range that goes down or an empty part
    bool valid = true;
    // more than maxTrackedIssues issues, counted before any range is expanded; `numbers` is then empty
    bool tooMany = false;
};
TrackedIssues parseTrackedIssues(const QString &text);

// the numbers for a summary: whole numbers in a row as "40-52", the others as written, at most `parts` parts
QString compactIssues(const QStringList &numbers, int parts = 8);

// the gaps from 1 to the highest whole number of `numbers`, as an Issues text ("1-3, 7"); empty without gaps or
// with more than maxTrackedIssues missing issues
QString missingIssueRanges(const QStringList &numbers);

// the numbers of the comics and missing comics of the folder
std::optional<QStringList> folderIssueNumbers(FolderId folderId, QSqlDatabase &db);
// the most common series, volume, and publisher of the folder's comics
DescriptiveComicMetadata folderRunMetadata(FolderId folderId, QSqlDatabase &db);

// In one transaction, a missing comic in the folder for each number that the folder does not have yet (as a comic
// or a missing comic), with the values of `patch` and the cover `coverKey`; gives how many were tracked
std::optional<int> trackIssuesInFolder(FolderId folderId, const QStringList &numbers, const DescriptiveMetadataPatch &patch, const std::optional<QString> &coverKey, QSqlDatabase &db);

#endif
