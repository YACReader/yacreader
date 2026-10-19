#ifndef CBL_MATCHER_H
#define CBL_MATCHER_H

#include "cbl_reader.h"
#include "library_ids.h"

#include <QHash>
#include <QList>
#include <QString>

#include <optional>
#include <variant>

class QSqlDatabase;

// the fields matching reads, from a comic_info or a placeholder
struct CblLibraryMetadata {
    QString series;
    QString number;
    QString volume;
    QString date;
    QString format;
    QString comicVineId;

    friend bool operator==(const CblLibraryMetadata &, const CblLibraryMetadata &) = default;
};

// deleted comics (no file) are included
struct CblLibraryComic : CblLibraryMetadata {
    ComicInfoId id;
    bool hasFile = false;

    friend bool operator==(const CblLibraryComic &, const CblLibraryComic &) = default;
};

struct CblLibraryPlaceholder : CblLibraryMetadata {
    PlaceholderId id;

    friend bool operator==(const CblLibraryPlaceholder &, const CblLibraryPlaceholder &) = default;
};

using CblCandidate = std::variant<CblLibraryComic, CblLibraryPlaceholder>;

// from the most to the least certain; the first level that finds a candidate is used
enum class CblMatchLevel {
    ComicVineId,
    SameSeries,
    SeriesWithoutVolume,
    LettersAndDigits,
    SeriesWithYear,
};

struct CblBookMatch {
    // what the import uses without asking the user
    std::optional<CblCandidate> accepted;
    // what fits the book at `level`, best first; the user chooses when nothing is accepted
    QList<CblCandidate> candidates;
    std::optional<CblMatchLevel> level;
};

// an accepted comic wins; else an accepted placeholder; the candidates list the comics, then the placeholders
class CblMatcher
{
public:
    CblMatcher(const QList<CblLibraryComic> &comics, const QList<CblLibraryPlaceholder> &placeholders = { });

    CblBookMatch match(const CblBook &book) const;

    // every comic_info with a series or a Comic Vine id; nothing if the query fails
    static std::optional<QList<CblLibraryComic>> loadLibraryComics(QSqlDatabase &db);
    // every placeholder with a series or a Comic Vine id; nothing if the query fails
    static std::optional<QList<CblLibraryPlaceholder>> loadLibraryPlaceholders(QSqlDatabase &db);

    struct Index {
        QHash<QString, QList<qsizetype>> byComicVineId;
        // per name level: series key and number -> positions
        QHash<QString, QList<qsizetype>> byName[4];
    };

private:
    QList<CblLibraryComic> comics;
    QList<CblLibraryPlaceholder> placeholders;
    Index comicIndex;
    Index placeholderIndex;
};

#endif
