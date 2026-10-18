#ifndef __COMICDB_H
#define __COMICDB_H

#include "comic_file.h"
#include "descriptive_comic_metadata.h"
#include "library_ids.h"

#include <QImage>
#include <QList>
#include <QMetaType>
#include <QPixmap>
#include <QVariant>

class ComicInfo : public DescriptiveComicMetadata
{
public:
    ComicInfo();

    void deleteMetadata();

    bool operator==(const ComicInfo &other) const { return id == other.id; }
    bool operator!=(const ComicInfo &other) const { return id != other.id; }

    // operator== compares only the id; this compares every column
    bool hasSameValues(const ComicInfo &other) const;

    // mandatory fields
    ComicInfoId id;
    bool read;
    bool edited;
    QString hash;

    int rating; // TODO_METADATA: change to float

    bool hasBeenOpened;

    // viewer
    int currentPage;
    int bookmark1;
    int bookmark2;
    int bookmark3;
    [[deprecated("Use imageFiltersJson")]] int brightness;
    [[deprecated("Use imageFiltersJson")]] int contrast;
    [[deprecated("Use imageFiltersJson")]] int gamma;
    //-----------------

    QVariant coverPage; // int
    QVariant numPages; // int

    QVariant lastTimeOpened; // integer/date
    QVariant coverSizeRatio; // h/w
    QVariant originalCoverSize; // string "WxH"

    QVariant added; // integer/date
    QVariant type; // enum

    QVariant imageFiltersJson; // string, JSON with image filters
    QVariant lastTimeImageFiltersSet; // integer/date, last time image filters were set
    QVariant lastTimeCoverSet; // integer/date, last time cover was set
    QVariant usesExternalCover; // bool, whether the cover is external or not
    QVariant lastTimeMetadataSet; // integer/date, last time metadata was set

    QPixmap getCover(const QString &basePath) const;

    QString getTypeString() const;

    static const QStringList &columns();
    static ComicInfo fromRecord(const QSqlRecord &record, const QString &alias);
    static ComicInfo fromQuery(const QSqlQuery &query, const QList<int> &columnIndexes);

    friend QDataStream &operator<<(QDataStream &stream, const ComicInfo &comicInfo);

    friend QDataStream &operator>>(QDataStream &stream, ComicInfo &comicInfo);
};

class ComicDB
{
public:
    ComicFile file;
    ComicInfo info;

    // return comic file name
    QString getFileName() const;

    // returns comic title if it isn't null or empty, in other case returns fileName
    QString getTitleOrFileName() const;

    // returns parent folder name

    // return the size of the file in bytes
    qulonglong getFileSize() const;

    QString getTitleIncludingNumber() const;
    QString getInfoTitle() const;

    QString toTXT() const;

    bool operator==(const ComicDB &other) const { return file.id == other.file.id; }

    struct ColumnIndexes {
        QList<int> file;
        QList<int> info;
    };

    static QString selectList(const QString &fileAlias, const QString &infoAlias);
    static ColumnIndexes columnIndexes(const QSqlRecord &record, const QString &fileAlias, const QString &infoAlias);
    static ComicDB fromRecord(const QSqlRecord &record, const QString &fileAlias, const QString &infoAlias);
    static ComicDB fromQuery(const QSqlQuery &query, const ColumnIndexes &columnIndexes);

    friend QDataStream &operator<<(QDataStream &, const ComicDB &);
    friend QDataStream &operator>>(QDataStream &, ComicDB &);
};

Q_DECLARE_METATYPE(ComicDB)

// the titles of ComicDB, also for a comic without a file; fileName is the fallback when there is no title
QString titleOrFileName(const DescriptiveComicMetadata &info, const QString &fileName);
QString titleIncludingNumber(const DescriptiveComicMetadata &info, const QString &fileName);
QString infoTitle(const DescriptiveComicMetadata &info, const QString &fileName);
// the name of the folder that holds `path` (a path in the library, with '/')
QString parentFolderName(const QString &path);

#endif
