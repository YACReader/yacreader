#ifndef COMIC_VINE_CLIENT_H
#define COMIC_VINE_CLIENT_H

#include "http_worker.h"

#include <QObject>
#include <QSettings>

class ComicVineClient : public QObject
{
    Q_OBJECT
public:
    explicit ComicVineClient(QObject *parent = nullptr);
    ~ComicVineClient() override;

signals:
    void searchResult(QString);
    void seriesDetail(QString); //JSON
    void comicDetail(QString); //JSON
    void seriesCover(const QByteArray &);
    void comicCover(const QByteArray &);
    void volumeComicsInfo(QString);
    void timeOut();
    void finished();
public slots:
    void search(const QString &query, int page = 1);
    void getSeriesDetail(const QString &id);
    void getSeriesCover(const QString &url);
    void getVolumeComicsInfo(const QString &idVolume, int page = 1);
    void getAllVolumeComicsInfo(const QString &idVolume);
    QByteArray getComicDetail(const QString &id, bool &outError, bool &outTimeout);
    void getComicCover(const QString &url);

    void getComicId(const QString &id, int comicNumber);
    void getCoverURL(const QString &id);
    void getComicDetailAsync(const QString &id);
protected slots:
    void proccessVolumesSearchData(const QByteArray &data);
    void proccessSeriesDetailData(const QByteArray &data);
    void processVolumeComicsInfo(const QByteArray &data);
    void proccessComicDetailData(const QByteArray &data);

protected:
    QSettings *settings;
    QString baseURL;
};
#endif // COMIC_VINE_CLIENT_H
