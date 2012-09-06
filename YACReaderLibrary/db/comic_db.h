#ifndef __COMIC_H
#define __COMIC_H

#include "library_item.h"
#include <QSqlDatabase>
#include <QList>
#include <QPixmap>
#include <QImage>

class ComicInfo
{
public:
	ComicInfo();
	ComicInfo(const ComicInfo & comicInfo);
	~ComicInfo();

	bool load(QString hash, QSqlDatabase & db);
	qulonglong insert(QSqlDatabase & db);
	void removeFromDB(QSqlDatabase & db);
	void update(QSqlDatabase & db);
	void updateRead(QSqlDatabase & db);

	qulonglong id;
	bool read;
	bool edited;
	QString hash;
	bool existOnDb;

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

private:
	void setField(const QString & name, QString * & field, QSqlRecord & record);
	void setField(const QString & name, int * & field, QSqlRecord & record);
	void setField(const QString & name, bool * & field, QSqlRecord & record);

	void bindField(const QString & name, QString * field, QSqlQuery & query);
	void bindField(const QString & name, int * field, QSqlQuery & query);
	void bindField(const QString & name, bool * field, QSqlQuery & query);

	void setValue(QString * & field, const QString & value);
	void setValue(int * & field, int value);
	void setValue(bool * & field, bool value);

	void copyField(QString * & field, const QString * value);
	void copyField(int * & field, int * value);
	void copyField(bool * & field, bool * value);
};

class ComicDB : public LibraryItem
{
private:
	bool _hasCover;
public:
	ComicDB();
	ComicDB(qulonglong cparentId, QString cname, QString cpath, QString chash, QSqlDatabase & database);
	//Comic(QString fn, QString fp):name(fn),path(fp),knownParent(false), knownId(false){};
	
	static QList<LibraryItem *> getComicsFromParent(qulonglong parentId, QSqlDatabase & db);
	bool isDir();

	bool load(qulonglong id, QSqlDatabase & db);
	qulonglong insert(QSqlDatabase & db);
	void removeFromDB(QSqlDatabase & db);
	void update(QSqlDatabase & db);
	bool hasCover() {return _hasCover;};
	
	ComicInfo info;
};


#endif