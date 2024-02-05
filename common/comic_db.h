#ifndef __COMICDB_H
#define __COMICDB_H

#include "library_item.h"
#include <QVariant>
#include <QList>
#include <QPixmap>
#include <QImage>
#include <QMetaType>

class ComicInfo : public QObject
{
    Q_OBJECT
public:
    ComicInfo();
    ComicInfo(const ComicInfo &comicInfo);
    ~ComicInfo();

    void deleteMetadata();

    ComicInfo &operator=(const ComicInfo &comicInfo);

    bool operator==(const ComicInfo &other) { return id == other.id; }
    bool operator!=(const ComicInfo &other) { return id != other.id; }

    // mandatory fields
    qulonglong id;
    bool read;
    bool edited;
    QString hash;
    bool existOnDb;

    int rating; // TODO_METADATA: change to float

    bool hasBeenOpened;

    // viewer
    int currentPage;
    int bookmark1;
    int bookmark2;
    int bookmark3;
    int brightness;
    int contrast;
    int gamma;
    //-----------------

    QVariant title; // string

    QVariant coverPage; // int
    QVariant numPages; // int

    QVariant number; // string (changed in 9.13 from int)
    QVariant isBis; // bool
    QVariant count; // int

    QVariant volume; // string
    QVariant storyArc; // string
    QVariant arcNumber; // string (changed in 9.13 from int)
    QVariant arcCount; // int

    QVariant genere; // string

    QVariant writer; // string
    QVariant penciller; // string
    QVariant inker; // string
    QVariant colorist; // string
    QVariant letterer; // string
    QVariant coverArtist; // string

    QVariant date; // string

    QVariant day;
    QVariant month;
    QVariant year;

    QVariant publisher; // string
    QVariant format; // string
    QVariant color; // bool
    QVariant ageRating; // string

    QVariant synopsis; // string
    QVariant characters; // string
    QVariant notes; // string

    QVariant comicVineID; // string

    QImage cover;

    QVariant lastTimeOpened; // integer/date
    QVariant coverSizeRatio; // h/w
    QVariant originalCoverSize; // string "WxH"

    QVariant added; // integer/date
    QVariant type; // enum
    QVariant editor; // string
    QVariant imprint; // string
    QVariant teams; // string/list
    QVariant locations; // string/list
    QVariant series; // string
    QVariant alternateSeries; // string
    QVariant alternateNumber; // string
    QVariant alternateCount; // int
    QVariant languageISO; // string
    QVariant seriesGroup; // string
    QVariant mainCharacterOrTeam; // string
    QVariant review; // string
    QVariant tags; // string/list

    QPixmap getCover(const QString &basePath);

    Q_INVOKABLE QStringList getWriters();
    Q_INVOKABLE QStringList getPencillers();
    Q_INVOKABLE QStringList getInkers();
    Q_INVOKABLE QStringList getColorists();
    Q_INVOKABLE QStringList getLetterers();
    Q_INVOKABLE QStringList getCoverArtists();
    Q_INVOKABLE QStringList getEditors();
    Q_INVOKABLE QStringList getImprint();

    Q_INVOKABLE QStringList getCharacters();

    Q_INVOKABLE QStringList getTeams();
    Q_INVOKABLE QStringList getLocations();

    Q_INVOKABLE QStringList getTags();

    Q_INVOKABLE QString getTypeString();

    Q_INVOKABLE QString getStoryArcInfoString();
    Q_INVOKABLE QString getAlternateSeriesString();

    friend QDataStream &operator<<(QDataStream &stream, const ComicInfo &comicInfo);

    friend QDataStream &operator>>(QDataStream &stream, ComicInfo &comicInfo);

    Q_PROPERTY(qulonglong id MEMBER id CONSTANT)
    Q_PROPERTY(bool read MEMBER read WRITE setRead NOTIFY readChanged)
    Q_PROPERTY(bool edited MEMBER edited CONSTANT)
    Q_PROPERTY(QString hash MEMBER hash CONSTANT)
    Q_PROPERTY(bool existOnDb MEMBER existOnDb CONSTANT)

    Q_PROPERTY(int rating MEMBER rating WRITE setRating NOTIFY ratingChanged)

    Q_PROPERTY(bool hasBeenOpened MEMBER hasBeenOpened CONSTANT)

    Q_PROPERTY(int currentPage MEMBER currentPage CONSTANT)
    Q_PROPERTY(int bookmark1 MEMBER bookmark1 CONSTANT)
    Q_PROPERTY(int bookmark2 MEMBER bookmark2 CONSTANT)
    Q_PROPERTY(int bookmark3 MEMBER bookmark3 CONSTANT)
    Q_PROPERTY(int brightness MEMBER brightness CONSTANT)
    Q_PROPERTY(int contrast MEMBER contrast CONSTANT)
    Q_PROPERTY(int gamma MEMBER gamma CONSTANT)

    Q_PROPERTY(QVariant title MEMBER title CONSTANT)

    Q_PROPERTY(QVariant coverPage MEMBER coverPage CONSTANT)
    Q_PROPERTY(QVariant numPages MEMBER numPages CONSTANT)

    Q_PROPERTY(QVariant number MEMBER number CONSTANT)
    Q_PROPERTY(QVariant isBis MEMBER isBis CONSTANT)
    Q_PROPERTY(QVariant count MEMBER count CONSTANT)

    Q_PROPERTY(QVariant volume MEMBER volume CONSTANT)
    Q_PROPERTY(QVariant storyArc MEMBER storyArc CONSTANT)
    Q_PROPERTY(QVariant arcNumber MEMBER arcNumber CONSTANT)
    Q_PROPERTY(QVariant arcCount MEMBER arcCount CONSTANT)

    Q_PROPERTY(QVariant genere MEMBER genere CONSTANT)

    Q_PROPERTY(QVariant writer MEMBER writer CONSTANT)
    Q_PROPERTY(QVariant penciller MEMBER penciller CONSTANT)
    Q_PROPERTY(QVariant inker MEMBER inker CONSTANT)
    Q_PROPERTY(QVariant colorist MEMBER colorist CONSTANT)
    Q_PROPERTY(QVariant letterer MEMBER letterer CONSTANT)
    Q_PROPERTY(QVariant coverArtist MEMBER coverArtist CONSTANT)

    Q_PROPERTY(QVariant date MEMBER date CONSTANT)
    Q_PROPERTY(QVariant day MEMBER day CONSTANT)
    Q_PROPERTY(QVariant month MEMBER month CONSTANT)
    Q_PROPERTY(QVariant year MEMBER year CONSTANT)
    Q_PROPERTY(QVariant publisher MEMBER publisher CONSTANT)
    Q_PROPERTY(QVariant format MEMBER format CONSTANT)
    Q_PROPERTY(QVariant color MEMBER color CONSTANT)
    Q_PROPERTY(QVariant ageRating MEMBER ageRating CONSTANT)

    Q_PROPERTY(QVariant synopsis MEMBER synopsis CONSTANT)
    Q_PROPERTY(QVariant characters MEMBER characters CONSTANT)
    Q_PROPERTY(QVariant notes MEMBER notes CONSTANT)

    Q_PROPERTY(QVariant comicVineID MEMBER comicVineID CONSTANT)

    Q_PROPERTY(QImage cover MEMBER cover CONSTANT)

    Q_PROPERTY(QVariant lastTimeOpened MEMBER lastTimeOpened CONSTANT)

    Q_PROPERTY(QVariant coverSizeRatio MEMBER coverSizeRatio CONSTANT)
    Q_PROPERTY(QVariant originalCoverSize MEMBER originalCoverSize CONSTANT)

    Q_PROPERTY(QVariant added MEMBER added CONSTANT)
    Q_PROPERTY(QVariant type MEMBER type CONSTANT)
    Q_PROPERTY(QVariant editor MEMBER editor CONSTANT)
    Q_PROPERTY(QVariant imprint MEMBER imprint CONSTANT)
    Q_PROPERTY(QVariant teams MEMBER teams CONSTANT)
    Q_PROPERTY(QVariant locations MEMBER locations CONSTANT)
    Q_PROPERTY(QVariant series MEMBER series CONSTANT)
    Q_PROPERTY(QVariant alternateSeries MEMBER alternateSeries CONSTANT)
    Q_PROPERTY(QVariant alternateNumber MEMBER alternateNumber CONSTANT)
    Q_PROPERTY(QVariant alternateCount MEMBER alternateCount CONSTANT)
    Q_PROPERTY(QVariant languageISO MEMBER languageISO CONSTANT)
    Q_PROPERTY(QVariant seriesGroup MEMBER seriesGroup CONSTANT)
    Q_PROPERTY(QVariant mainCharacterOrTeam MEMBER mainCharacterOrTeam CONSTANT)
    Q_PROPERTY(QVariant review MEMBER review CONSTANT)
    Q_PROPERTY(QVariant tags MEMBER tags CONSTANT)

    //-new properties, not loaded from the DB automatically
    bool isFavorite;
    Q_PROPERTY(bool isFavorite MEMBER isFavorite WRITE setFavorite NOTIFY favoriteChanged)

    // setters, used in QML only for now
    void setRead(bool r);
    void setRating(int r);
    void setFavorite(bool f);

private:
signals:
    void readChanged();
    void ratingChanged();
    void favoriteChanged();
};

class ComicDB : public LibraryItem
{
    Q_OBJECT
public:
    ComicDB();
    ComicDB(const ComicDB &comicDB);

    bool isDir() const;

    bool _hasCover;

    bool hasCover() { return _hasCover; }

    // return comic file name
    QString getFileName() const;

    // returns comic title if it isn't null or empty, in other case returns fileName
    Q_INVOKABLE QString getTitleOrFileName() const;

    // returns parent folder name
    QString getParentFolderName() const;

    // return the size of the file in bytes
    Q_INVOKABLE qulonglong getFileSize() const;

    Q_INVOKABLE QString getTitleIncludingNumber() const;
    Q_INVOKABLE QString getInfoTitle() const;

    QString toTXT();

    ComicInfo info;
    Q_PROPERTY(ComicInfo info MEMBER info)

    ComicDB &operator=(const ComicDB &other);
    bool operator==(const ComicDB &other) const { return id == other.id; }

    friend QDataStream &operator<<(QDataStream &, const ComicDB &);
    friend QDataStream &operator>>(QDataStream &, ComicDB &);
};

Q_DECLARE_METATYPE(ComicDB)

#endif
