#ifndef CBL_MATCHER_H
#define CBL_MATCHER_H

#include "cbl_reader.h"
#include "library_ids.h"

#include <QHash>
#include <QList>
#include <QString>

#include <optional>

class QSqlDatabase;

// a comic_info as matching sees it; deleted comics (no file) are included
struct CblLibraryComic {
    ComicInfoId id;
    bool hasFile = false;
    QString series;
    QString number;
    QString volume;
    QString date;
    QString format;
    QString comicVineId;

    friend bool operator==(const CblLibraryComic &, const CblLibraryComic &) = default;
};

// from the most to the least certain; the first level that finds a comic is used
enum class CblMatchLevel {
    ComicVineId,
    SameSeries,
    SeriesWithoutVolume,
    LettersAndDigits,
    SeriesWithYear,
};

struct CblBookMatch {
    // the comic the import uses without asking the user
    std::optional<CblLibraryComic> accepted;
    // the comics that fit the book at `level`, best first; the user chooses when nothing is accepted
    QList<CblLibraryComic> candidates;
    std::optional<CblMatchLevel> level;
};

class CblMatcher
{
public:
    explicit CblMatcher(const QList<CblLibraryComic> &comics);

    CblBookMatch match(const CblBook &book) const;

    // every comic_info with a series or a Comic Vine id; nothing if the query fails
    static std::optional<QList<CblLibraryComic>> loadLibraryComics(QSqlDatabase &db);

private:
    QList<CblLibraryComic> comics;
    QHash<QString, QList<qsizetype>> byComicVineId;
    // per name level: series key and number -> comics
    QHash<QString, QList<qsizetype>> byName[4];
};

#endif
