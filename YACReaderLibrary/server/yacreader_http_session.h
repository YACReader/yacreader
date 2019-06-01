#ifndef YACREADERHTTPSESSION_H
#define YACREADERHTTPSESSION_H

#include <QObject>

#include "comic.h"

class YACReaderHttpSession : public QObject
{
    Q_OBJECT
public:
    explicit YACReaderHttpSession(QObject *parent = nullptr);
    ~YACReaderHttpSession() override;

    void setComicsOnDevice(const QSet<QString> &set);
    void setComicOnDevice(const QString &hash);
    void setDownloadedComic(const QString &hash);
    bool isComicOnDevice(const QString &hash);
    bool isComicDownloaded(const QString &hash);
    QSet<QString> getComicsOnDevice();
    QSet<QString> getDownloadedComics();
    void clearComics();

    //current comic (import)
    qulonglong getCurrentComicId();
    Comic *getCurrentComic();
    void dismissCurrentComic();
    void setCurrentComic(qulonglong id, Comic *comic);

    //current comic (read)
    qulonglong getCurrentRemoteComicId();
    Comic *getCurrentRemoteComic();
    void dismissCurrentRemoteComic();
    void setCurrentRemoteComic(qulonglong id, Comic *comic);

    //device identification
    QString getDeviceType();
    QString getDisplayType();
    void setDeviceType(const QString &device);
    void setDisplayType(const QString &display);

    void clearNavigationPath();
    QPair<qulonglong, quint32> popNavigationItem();
    QPair<qulonglong, quint32> topNavigationItem();
    void pushNavigationItem(const QPair<qulonglong, quint32> &item);
    void updateTopItem(const QPair<qulonglong, quint32> &item);

    //TODO replace QPair by a custom class for storing folderId, page and folderName(save some DB accesses)
    QStack<QPair<qulonglong, quint32>> getNavigationPath();

signals:

public slots:

private:
    QSet<QString> comicsOnDevice;
    QSet<QString> downloadedComics;

    QString device;
    QString display;

    qulonglong comicId;
    qulonglong remoteComicId;
    Comic *comic;
    Comic *remoteComic;

    QStack<QPair<qulonglong, quint32>> navigationPath; /* folder_id, page_number */
};

#endif // YACREADERHTTPSESSION_H
