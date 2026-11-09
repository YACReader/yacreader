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
    // the path of one of its comics (the lowest id); empty when it has no file
    QString path;
    QString hash;

    friend bool operator==(const CblLibraryComic &, const CblLibraryComic &) = default;
};

struct CblLibraryPlaceholder : CblLibraryMetadata {
    PlaceholderId id;
    QString coverKey;

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
    // the comics of the series that the name levels find for the book series with any number (the same series, then
    // without the volume, then letters and digits); only those of the book's volume when there are any
    QList<CblLibraryComic> closestSeries(const CblBook &book) const;

    // every comic_info with a series or a Comic Vine id; nothing if the query fails
    static std::optional<QList<CblLibraryComic>> loadLibraryComics(QSqlDatabase &db);
    // every placeholder with a series or a Comic Vine id; nothing if the query fails
    static std::optional<QList<CblLibraryPlaceholder>> loadLibraryPlaceholders(QSqlDatabase &db);

    struct Index {
        QHash<QString, QList<qsizetype>> byComicVineId;
        // per name level: series key and number -> positions
        QHash<QString, QList<qsizetype>> byName[4];
        // per series level (same series, without the volume, letters and digits): series key -> positions
        QHash<QString, QList<qsizetype>> bySeries[3];
    };

private:
    QList<CblLibraryComic> comics;
    QList<CblLibraryPlaceholder> placeholders;
    Index comicIndex;
    Index placeholderIndex;
};

// the values that matching compares, for the UI
QString cblComicVineIssueId(const CblBook &book);
// the year, else the year of the cover date; 0 when unknown
int cblBookYear(const CblBook &book);
// the year of a dd/mm/yyyy date; 0 when unknown
int cblYearOfDate(const QString &date);
// the volume, or the year in a "Series (Year)" name
QString cblEffectiveVolume(const CblLibraryMetadata &metadata);
// only the case and leading zeros do not count
bool cblSameNumber(const QString &a, const QString &b);

#endif
