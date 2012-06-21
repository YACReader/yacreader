#ifndef __COMIC_H
#define __COMIC_H

#include "library_item.h"
#include <QSqlDatabase>
#include <QList>

class ComicInfo
{
public:
	ComicInfo();

	bool load(QString hash, QSqlDatabase & db);
	qulonglong insert(QSqlDatabase & db);
	void removeFromDB(QSqlDatabase & db);
	void update(QSqlDatabase & db);

	qulonglong id;
	bool read;
	bool edited;
	QString hash;

	QString title;
	int pages;

	int coverPage;

	int number;
	bool isBis;
	int count;

	QString volume;
	QString storyArc;
	int arcNumber;
	int arcCount;

	QString genere;

	QString writer;
	QString penciller;
	QString inker;
	QString colorist;
	QString letterer;
	QString coverArtist;

	QString date;
	QString publisher;
	QString format;
	bool color;
	QString ageRating;

	QString synopsis;
	QString characters;
	QString notes;


	bool existOnDb;
};

class Comic : public LibraryItem
{
private:
	bool _hasCover;
public:
	Comic();
	Comic(qulonglong cparentId, QString cname, QString cpath, QString chash, QSqlDatabase & database);
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