#ifndef COMIC_VINE_ALL_VOLUME_COMICS_RETRIEVER_H
#define COMIC_VINE_ALL_VOLUME_COMICS_RETRIEVER_H

#include <QObject>

class ComicVineAllVolumeComicsRetriever : public QObject
{
    Q_OBJECT
public:
    explicit ComicVineAllVolumeComicsRetriever(const QString &volumeURLString, QObject *parent = nullptr);
    void getAllVolumeComics();

protected:
    void getAllVolumeComics(const int range);
signals:
    void allVolumeComicsInfo(QString json);
    void finished();
    void timeOut();
protected slots:
    void appendVolumeComicsInfo(const QByteArray &data);

protected:
    QString volumeURLString;
    QList<QByteArray> jsonResponses;

    QString consolidateJSON();
};

#endif // COMIC_VINE_ALL_VOLUME_COMICS_RETRIEVER_H
