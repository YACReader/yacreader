#ifndef __COMICDB_H
#define __COMICDB_H

#include "library_item.h"
#include <QVariant>
#include <QList>
#include <QPixmap>
#include <QImage>
#include <QMetaType>

class ComicInfo
{
public:
	ComicInfo();
	ComicInfo(const ComicInfo & comicInfo);
	~ComicInfo();

	ComicInfo & operator=(const ComicInfo & comicInfo);

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

	
    QVariant title;//string
	
    QVariant coverPage;//int
    QVariant numPages;//int

    QVariant number;//int
    QVariant isBis;//bool
    QVariant count;//int

    QVariant volume;//string
    QVariant storyArc;//string
    QVariant arcNumber;//int
    QVariant arcCount;//int

    QVariant genere;//string

    QVariant writer;//string
    QVariant penciller;//string
    QVariant inker;//string
    QVariant colorist;//string
    QVariant letterer;//string
    QVariant coverArtist;//string

    QVariant date;//string
    QVariant publisher;//string
    QVariant format;//string
    QVariant color;//bool
    QVariant ageRating;//string

    QVariant synopsis;//string
    QVariant characters;//string
    QVariant notes;//string

    QVariant comicVineID;//string

	QImage cover;

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

	QPixmap getCover(const QString & basePath);

	friend QDataStream &operator<<(QDataStream & stream, const ComicInfo & comicInfo);

	friend QDataStream &operator>>(QDataStream & stream, ComicInfo & comicInfo);

private:

};

class ComicDB : public LibraryItem
{
public:
	ComicDB();
	
	bool isDir();
	
	bool _hasCover;

	bool hasCover() {return _hasCover;};

	//return comic file name
	QString getFileName() const;

	//returns comic title if it isn't null or empty, in other case returns fileName
	QString getTitleOrFileName() const;

	//returns parent folder name
	QString getParentFolderName() const;

	QString toTXT();
	
	ComicInfo info;

	bool operator==(const ComicDB & other){return id == other.id;};

	friend QDataStream &operator<<(QDataStream &, const ComicDB &);
	friend QDataStream &operator>>(QDataStream &, ComicDB &);
};

Q_DECLARE_METATYPE(ComicDB);

#endif
