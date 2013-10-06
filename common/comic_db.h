#ifndef __COMICDB_H
#define __COMICDB_H

#include "library_item.h"
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

	

	QString * title;
	
	int * coverPage;
	int * numPages;

	int * number;
	bool * isBis;
	int * count;

	QString * volume;
	QString * storyArc;
	int * arcNumber;
	int * arcCount;

	QString * genere;

	QString * writer;
	QString * penciller;
	QString * inker;
	QString * colorist;
	QString * letterer;
	QString * coverArtist;

	QString * date;
	QString * publisher;
	QString * format;
	bool * color;
	QString * ageRating;

	QString * synopsis;
	QString * characters;
	QString * notes;

	QImage cover;

	void setTitle(QString value);

	void setCoverPage(int value);
	void setNumPages(int value);

	void setNumber(int value);
	void setIsBis(bool value);
	void setCount(int value);

	void setVolume(QString value);
	void setStoryArc(QString value);
	void setArcNumber(int value);
	void setArcCount(int value);

	void setGenere(QString value);

	void setWriter(QString value);
	void setPenciller(QString value);
	void setInker(QString value);
	void setColorist(QString value);
	void setLetterer(QString value);
	void setCoverArtist(QString value);

	void setDate(QString value);
	void setPublisher(QString value);
	void setFormat(QString value);
	void setColor(bool value);
	void setAgeRating(QString value);

	void setSynopsis(QString value);
	void setCharacters(QString value);
	void setNotes(QString value);

	QPixmap getCover(const QString & basePath);

	friend QDataStream &operator<<(QDataStream & stream, const ComicInfo & comicInfo);

	friend QDataStream &operator>>(QDataStream & stream, ComicInfo & comicInfo);

private:
	void setValue(QString * & field, const QString & value);
	void setValue(int * & field, int value);
	void setValue(bool * & field, bool value);

	void copyField(QString * & field, const QString * value);
	void copyField(int * & field, int * value);
	void copyField(bool * & field, bool * value);
};

class ComicDB : public LibraryItem
{
public:
	ComicDB();
	
	bool isDir();
	
	bool _hasCover;

	bool hasCover() {return _hasCover;};

	//returns comic title if it isn't null or empty, in other case returns fileName
	QString getTitleOrPath();

	//returns parent folder name
	QString getParentFolderName();

	QString toTXT();
	
	ComicInfo info;

	bool operator==(const ComicDB & other){return id == other.id;};

	friend QDataStream &operator<<(QDataStream &, const ComicDB &);
	friend QDataStream &operator>>(QDataStream &, ComicDB &);
};

Q_DECLARE_METATYPE(ComicDB);

#endif
