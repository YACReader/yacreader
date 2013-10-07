#ifndef COMIC_VINE_CLIENT_H
#define COMIC_VINE_CLIENT_H

#include "http_worker.h"

#include <QObject>

class ComicVineClient : public QObject
{
	Q_OBJECT
public:
	explicit ComicVineClient(QObject *parent = 0);
	
signals:
	void searchResult(QString);
public slots:
	void search(const QString & query, int page = 0);
	void getSeriesDetail(const QString & id);
	void getComicIds(const QString & id, int page = 0);
	void getComicId(const QString & id, int comicNumber);
	void getComicDetail(const QString & id);
	void getCoverURL(const QString & id);
protected slots:
	void proccessVolumesSearchData(const QByteArray & data);
	void queryTimeOut();
	
};
#endif // COMIC_VINE_CLIENT_H
