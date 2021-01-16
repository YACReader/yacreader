#ifndef __COMICDB_H
#define __COMICDB_H

#include "library_item.h"
#include <QVariant>
#include <QList>
#include <QPixmap>
#include <QImage>
#include <QMetaType>

typedef QPair<QString, QString> YACReaderComicInfoPair;
Q_DECLARE_METATYPE(YACReaderComicInfoPair)

class ComicInfo : public QObject
{
    Q_OBJECT
public:
    ComicInfo();
    ComicInfo(const ComicInfo &comicInfo);
    ~ComicInfo();

    ComicInfo &operator=(const ComicInfo &comicInfo);

    bool operator==(const ComicInfo &other) { return id == other.id; }
    bool operator!=(const ComicInfo &other) { return id != other.id; }

    //mandatory fields
    qulonglong id;
    bool read;
    bool edited;
    QString hash;
    bool existOnDb;

    int rating;

    bool hasBeenOpened;

    //viewer
    int currentPage;
    int bookmark1;
    int bookmark2;
    int bookmark3;
    int brightness;
    int contrast;
    int gamma;
    //-----------------

    QVariant title; //string

    QVariant coverPage; //int
    QVariant numPages; //int

    QVariant number; //int
    QVariant isBis; //bool
    QVariant count; //int

    QVariant volume; //string
    QVariant storyArc; //string
    QVariant arcNumber; //int
    QVariant arcCount; //int

    QVariant genere; //string

    QVariant writer; //string
    QVariant penciller; //string
    QVariant inker; //string
    QVariant colorist; //string
    QVariant letterer; //string
    QVariant coverArtist; //string

    QVariant date; //string
    QVariant publisher; //string
    QVariant format; //string
    QVariant color; //bool
    QVariant ageRating; //string
    QVariant manga; //bool

    QVariant synopsis; //string
    QVariant characters; //string
    QVariant notes; //string

    QVariant comicVineID; //string

    QImage cover;

    QVariant lastTimeOpened; //integer/date
    QVariant coverSizeRatio; //h/w
    QVariant originalCoverSize; //string "WxH"

    /*void setTitle(QVariant value);

    void setCoverPage(QVariant value);
    void setNumPages(QVariant value);

    void setNumber(QVariant value);
    void setIsBis(QVariant value);
    void setCount(QVariant value);

    void setVolume(QVariant value);
    void setStoryArc(QVariant value);
    void setArcNumber(QVariant value);
    void setArcCount(QVariant value);

    void setGenere(QVariant value);

    void setWriter(QVariant value);
    void setPenciller(QVariant value);
    void setInker(QVariant value);
    void setColorist(QVariant value);
    void setLetterer(QVariant value);
    void setCoverArtist(QVariant value);

    void setDate(QVariant value);
    void setPublisher(QVariant value);
    void setFormat(QVariant value);
    void setColor(QVariant value);
    void setAgeRating(QVariant value);

    void setSynopsis(QVariant value);
    void setCharacters(QVariant value);
    void setNotes(QVariant value);*/

    QPixmap getCover(const QString &basePath);

    Q_INVOKABLE QStringList getWriters();
    Q_INVOKABLE QStringList getPencillers();
    Q_INVOKABLE QStringList getInkers();
    Q_INVOKABLE QStringList getColorists();
    Q_INVOKABLE QStringList getLetterers();
    Q_INVOKABLE QStringList getCoverArtists();

    Q_INVOKABLE QStringList getCharacters();

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
    Q_PROPERTY(QVariant publisher MEMBER publisher CONSTANT)
    Q_PROPERTY(QVariant format MEMBER format CONSTANT)
    Q_PROPERTY(QVariant color MEMBER color CONSTANT)
    Q_PROPERTY(QVariant ageRating MEMBER ageRating CONSTANT)
    Q_PROPERTY(QVariant manga MEMBER manga CONSTANT)

    Q_PROPERTY(QVariant synopsis MEMBER synopsis CONSTANT)
    Q_PROPERTY(QVariant characters MEMBER characters CONSTANT)
    Q_PROPERTY(QVariant notes MEMBER notes CONSTANT)

    Q_PROPERTY(QVariant comicVineID MEMBER comicVineID CONSTANT)

    Q_PROPERTY(QImage cover MEMBER cover CONSTANT)

    Q_PROPERTY(QVariant lastTimeOpened MEMBER lastTimeOpened CONSTANT)

    Q_PROPERTY(QVariant coverSizeRatio MEMBER coverSizeRatio CONSTANT)
    Q_PROPERTY(QVariant originalCoverSize MEMBER originalCoverSize CONSTANT)

    //-new properties, not loaded from the DB automatically
    bool isFavorite;
    Q_PROPERTY(bool isFavorite MEMBER isFavorite WRITE setFavorite NOTIFY favoriteChanged)

    //setters, used in QML only by now
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

    //return comic file name
    QString getFileName() const;

    //returns comic title if it isn't null or empty, in other case returns fileName
    Q_INVOKABLE QString getTitleOrFileName() const;

    //returns parent folder name
    QString getParentFolderName() const;

    //return the size of the file in bytes
    Q_INVOKABLE qulonglong getFileSize() const;

    Q_INVOKABLE QString getTitleIncludingNumber() const;

    QString toTXT();

    ComicInfo info;
    Q_PROPERTY(ComicInfo info MEMBER info)

    ComicDB &operator=(const ComicDB &other);
    bool operator==(const ComicDB &other) { return id == other.id; }

    friend QDataStream &operator<<(QDataStream &, const ComicDB &);
    friend QDataStream &operator>>(QDataStream &, ComicDB &);
};

Q_DECLARE_METATYPE(ComicDB)

#endif
