#ifndef FOLDER_PLACEHOLDER_MATCH_H
#define FOLDER_PLACEHOLDER_MATCH_H

#include "library_ids.h"

#include <QHash>
#include <QString>
#include <QStringList>

#include <optional>

class ComicDB;
class QSqlDatabase;
struct ComicMatchFiles;
struct DescriptiveComicMetadata;

// an issue number without leading zeros or case; two issues are the same when their keys are
QString issueNumberKey(const QString &number);
bool sameIssueNumber(const QString &a, const QString &b);

// the comicVineID, or the id that ComicTagger writes in the notes ("ComicVine [CVDB<id>]")
QString comicVineIdOf(const DescriptiveComicMetadata &metadata);

// the issue number of a file name by the comic rules of Kavita; nothing for a special (annual, one-shot, TPB...) or
// a name without a number
std::optional<QString> issueNumberInFileName(const QString &fileName);

// where the file names of a folder keep their issue number, learned from its comics with a number
class FileNamePattern
{
public:
    // file name and issue number of each comic
    explicit FileNamePattern(const QList<QPair<QString, QString>> &numberedFiles);

    // there is at least one trusted prefix
    bool isLearned() const;
    // the number after a trusted prefix; nothing when the name fits none, or fits several with other numbers
    std::optional<QString> issueNumber(const QString &fileName) const;

private:
    struct Observations {
        int agree = 0;
        int disagree = 0;
    };
    QHash<QString, Observations> prefixes;

    bool isTrusted(const QString &prefix) const;
};

// No transaction. When `comic` (a new file of its folder) is exactly one of the missing comics placed in its folder,
// the comic gets it (matchPlaceholderToComic) and its id is given back. Rules in order: the Comic Vine id, the series
// and number of the metadata, and when the metadata has no number, the number in the file name (the folder's
// pattern, or Kavita's rules when the folder has none).
std::optional<PlaceholderId> matchNewComicToMissingComic(const ComicDB &comic, const QString &libraryDataPath, QSqlDatabase &db, ComicMatchFiles &files);

#endif
