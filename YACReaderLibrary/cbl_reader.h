#ifndef CBL_READER_H
#define CBL_READER_H

#include <QList>
#include <QString>
#include <QStringList>

#include <optional>
#include <variant>

class QIODevice;

// CBL reading lists come as ComicRack XML (.cbl, no specification) or as JSON (json-cbl-standard 1.0);
// both give the same model, and fields a format does not have stay empty

// the ids of a book in one provider database, as the file names it ("cv", "comicvine", "metron", "gcd"…)
struct CblDatabaseIds {
    QString name;
    QString seriesId;
    QString issueId;

    friend bool operator==(const CblDatabaseIds &, const CblDatabaseIds &) = default;
};

// every value is the source text: numbers like "-1", "23.2", "½", or "1.MU", and years like "0", stay as they are
struct CblBook {
    QString series;
    QString number;
    // XML Volume; JSON seriesStartYear
    QString volume;
    // XML Year; JSON the year of issueCoverDate
    QString year;
    // JSON issueCoverDate, as written (YYYY-MM-DD)
    QString coverDate;
    // XML Format
    QString format;
    // JSON issueType
    QString issueType;
    // ComicRack's Id and FileName
    QString id;
    QString fileName;
    QList<CblDatabaseIds> databases;
    // the book as in the file: the <Book> element, or the JSON issue object (written again, with sorted keys)
    QString source;

    friend bool operator==(const CblBook &, const CblBook &) = default;
};

// a link to another reading list file, by its UUID
struct CblRelationship {
    QString name;
    QString uuid;
    // "previous" or "following"
    QString relationship;

    friend bool operator==(const CblRelationship &, const CblRelationship &) = default;
};

struct CblSource {
    QString name;
    QString url;
    QString id;

    friend bool operator==(const CblSource &, const CblSource &) = default;
};

struct CblReadingList {
    enum Format {
        Xml,
        Json,
    };

    Format format = Xml;
    // JSON fileDetails.UUID; the same list file keeps it across versions
    QString uuid;
    QString name;
    // XML Summary; JSON description
    QString description;
    // XML NumIssues; it can differ from the number of books
    std::optional<int> sourceCount;
    std::optional<int> startYear;
    std::optional<int> startMonth;
    std::optional<int> endYear;
    std::optional<int> endMonth;
    QString publisher;
    QString imprint;
    // "master", "universal", "team", "character", "story"…
    QString type;
    QStringList tags;
    QStringList coverImageUrls;
    QList<CblRelationship> relationships;
    QList<CblSource> sources;
    QString notes;
    QList<CblBook> books;

    friend bool operator==(const CblReadingList &, const CblReadingList &) = default;
};

struct CblReadError {
    enum Kind {
        CannotOpen,
        Malformed,
        NotAReadingList,
    };

    Kind kind;
    // the reader's message, for logs; the UI shows a text for the kind
    QString detail;
    qint64 line = 0;
    qint64 column = 0;
};

using CblReadResult = std::variant<CblReadingList, CblReadError>;

// the format is found from the content: '{' starts JSON, anything else is read as XML
CblReadResult readCbl(QIODevice &device);
CblReadResult readCbl(const QString &filePath);

#endif
