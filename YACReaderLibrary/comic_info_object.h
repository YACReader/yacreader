#ifndef COMIC_INFO_OBJECT_H
#define COMIC_INFO_OBJECT_H

#include "comic_db.h"
#include "comic_record.h"

#include <QObject>

class ComicInfoObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(qulonglong id READ id CONSTANT)
    Q_PROPERTY(bool read READ read WRITE setRead NOTIFY readChanged)
    Q_PROPERTY(bool edited READ edited CONSTANT)
    Q_PROPERTY(QString hash READ hash CONSTANT)

    Q_PROPERTY(int rating READ rating WRITE setRating NOTIFY ratingChanged)

    Q_PROPERTY(bool hasBeenOpened READ hasBeenOpened CONSTANT)

    Q_PROPERTY(int currentPage READ currentPage CONSTANT)
    Q_PROPERTY(int bookmark1 READ bookmark1 CONSTANT)
    Q_PROPERTY(int bookmark2 READ bookmark2 CONSTANT)
    Q_PROPERTY(int bookmark3 READ bookmark3 CONSTANT)
    Q_PROPERTY(int brightness READ brightness CONSTANT)
    Q_PROPERTY(int contrast READ contrast CONSTANT)
    Q_PROPERTY(int gamma READ gamma CONSTANT)

    Q_PROPERTY(QVariant title READ title CONSTANT)
    Q_PROPERTY(QVariant coverPage READ coverPage CONSTANT)
    Q_PROPERTY(QVariant numPages READ numPages CONSTANT)
    Q_PROPERTY(QVariant number READ number CONSTANT)
    Q_PROPERTY(QVariant isBis READ isBis CONSTANT)
    Q_PROPERTY(QVariant count READ count CONSTANT)
    Q_PROPERTY(QVariant volume READ volume CONSTANT)
    Q_PROPERTY(QVariant storyArc READ storyArc CONSTANT)
    Q_PROPERTY(QVariant arcNumber READ arcNumber CONSTANT)
    Q_PROPERTY(QVariant arcCount READ arcCount CONSTANT)
    Q_PROPERTY(QVariant genere READ genere CONSTANT)
    Q_PROPERTY(QVariant writer READ writer CONSTANT)
    Q_PROPERTY(QVariant penciller READ penciller CONSTANT)
    Q_PROPERTY(QVariant inker READ inker CONSTANT)
    Q_PROPERTY(QVariant colorist READ colorist CONSTANT)
    Q_PROPERTY(QVariant letterer READ letterer CONSTANT)
    Q_PROPERTY(QVariant coverArtist READ coverArtist CONSTANT)
    Q_PROPERTY(QVariant date READ date CONSTANT)
    Q_PROPERTY(QVariant publisher READ publisher CONSTANT)
    Q_PROPERTY(QVariant format READ format CONSTANT)
    Q_PROPERTY(QVariant color READ color CONSTANT)
    Q_PROPERTY(QVariant ageRating READ ageRating CONSTANT)
    Q_PROPERTY(QVariant synopsis READ synopsis CONSTANT)
    Q_PROPERTY(QVariant characters READ characters CONSTANT)
    Q_PROPERTY(QVariant notes READ notes CONSTANT)
    Q_PROPERTY(QVariant comicVineID READ comicVineID CONSTANT)
    Q_PROPERTY(QVariant lastTimeOpened READ lastTimeOpened CONSTANT)
    Q_PROPERTY(QVariant coverSizeRatio READ coverSizeRatio CONSTANT)
    Q_PROPERTY(QVariant originalCoverSize READ originalCoverSize CONSTANT)
    Q_PROPERTY(QVariant added READ added CONSTANT)
    Q_PROPERTY(QVariant type READ type CONSTANT)
    Q_PROPERTY(QVariant editor READ editor CONSTANT)
    Q_PROPERTY(QVariant imprint READ imprint CONSTANT)
    Q_PROPERTY(QVariant teams READ teams CONSTANT)
    Q_PROPERTY(QVariant locations READ locations CONSTANT)
    Q_PROPERTY(QVariant series READ series CONSTANT)
    Q_PROPERTY(QVariant alternateSeries READ alternateSeries CONSTANT)
    Q_PROPERTY(QVariant alternateNumber READ alternateNumber CONSTANT)
    Q_PROPERTY(QVariant alternateCount READ alternateCount CONSTANT)
    Q_PROPERTY(QVariant languageISO READ languageISO CONSTANT)
    Q_PROPERTY(QVariant seriesGroup READ seriesGroup CONSTANT)
    Q_PROPERTY(QVariant mainCharacterOrTeam READ mainCharacterOrTeam CONSTANT)
    Q_PROPERTY(QVariant review READ review CONSTANT)
    Q_PROPERTY(QVariant tags READ tags CONSTANT)
    Q_PROPERTY(QVariant imageFiltersJson READ imageFiltersJson CONSTANT)
    Q_PROPERTY(QVariant lastTimeImageFiltersSet READ lastTimeImageFiltersSet CONSTANT)
    Q_PROPERTY(QVariant lastTimeCoverSet READ lastTimeCoverSet CONSTANT)
    Q_PROPERTY(QVariant usesExternalCover READ usesExternalCover CONSTANT)
    Q_PROPERTY(QVariant lastTimeMetadataSet READ lastTimeMetadataSet CONSTANT)

    Q_PROPERTY(bool isFavorite READ isFavorite WRITE setFavorite NOTIFY favoriteChanged)
    // false for a comic whose file is no longer in the library
    Q_PROPERTY(bool available READ available CONSTANT)
    // false for a placeholder, which has no read state or rating
    Q_PROPERTY(bool canSetReadState READ canSetReadState CONSTANT)

public:
    explicit ComicInfoObject(QObject *parent = nullptr);
    ComicInfoObject(const ComicRecord &record, bool isFavorite, QObject *parent = nullptr);

    qulonglong id() const;
    bool read() const;
    bool edited() const;
    QString hash() const;
    int rating() const;
    bool hasBeenOpened() const;
    int currentPage() const;
    int bookmark1() const;
    int bookmark2() const;
    int bookmark3() const;
    int brightness() const;
    int contrast() const;
    int gamma() const;

    QVariant title() const;
    QVariant coverPage() const;
    QVariant numPages() const;
    QVariant number() const;
    QVariant isBis() const;
    QVariant count() const;
    QVariant volume() const;
    QVariant storyArc() const;
    QVariant arcNumber() const;
    QVariant arcCount() const;
    QVariant genere() const;
    QVariant writer() const;
    QVariant penciller() const;
    QVariant inker() const;
    QVariant colorist() const;
    QVariant letterer() const;
    QVariant coverArtist() const;
    QVariant date() const;
    QVariant publisher() const;
    QVariant format() const;
    QVariant color() const;
    QVariant ageRating() const;
    QVariant synopsis() const;
    QVariant characters() const;
    QVariant notes() const;
    QVariant comicVineID() const;
    QVariant lastTimeOpened() const;
    QVariant coverSizeRatio() const;
    QVariant originalCoverSize() const;
    QVariant added() const;
    QVariant type() const;
    QVariant editor() const;
    QVariant imprint() const;
    QVariant teams() const;
    QVariant locations() const;
    QVariant series() const;
    QVariant alternateSeries() const;
    QVariant alternateNumber() const;
    QVariant alternateCount() const;
    QVariant languageISO() const;
    QVariant seriesGroup() const;
    QVariant mainCharacterOrTeam() const;
    QVariant review() const;
    QVariant tags() const;
    QVariant imageFiltersJson() const;
    QVariant lastTimeImageFiltersSet() const;
    QVariant lastTimeCoverSet() const;
    QVariant usesExternalCover() const;
    QVariant lastTimeMetadataSet() const;

    bool isFavorite() const;
    bool available() const;
    bool canSetReadState() const;

    void setRead(bool read);
    void setRating(int rating);
    void setFavorite(bool isFavorite);

    Q_INVOKABLE QStringList getWriters() const;
    Q_INVOKABLE QStringList getPencillers() const;
    Q_INVOKABLE QStringList getInkers() const;
    Q_INVOKABLE QStringList getColorists() const;
    Q_INVOKABLE QStringList getLetterers() const;
    Q_INVOKABLE QStringList getCoverArtists() const;
    Q_INVOKABLE QStringList getEditors() const;
    Q_INVOKABLE QStringList getImprint() const;
    Q_INVOKABLE QStringList getCharacters() const;
    Q_INVOKABLE QStringList getTeams() const;
    Q_INVOKABLE QStringList getLocations() const;
    Q_INVOKABLE QStringList getTags() const;

    Q_INVOKABLE QString getTypeString() const;
    Q_INVOKABLE QString getStoryArcInfoString() const;
    Q_INVOKABLE QString getAlternateSeriesString() const;

    Q_INVOKABLE QString getTitleOrFileName() const;
    Q_INVOKABLE qulonglong getFileSize() const;
    Q_INVOKABLE QString getTitleIncludingNumber() const;
    Q_INVOKABLE QString getInfoTitle() const;

signals:
    void readChanged();
    void ratingChanged();
    void favoriteChanged();

private:
    static QString orMissingComic(const QString &title);

    ComicRecord record;
    // the last title fallback: the file name, or the last known one of a deleted comic
    QString fileName;
    bool isAvailable = true;
    bool favorite;
};

#endif
