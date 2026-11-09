#ifndef YACREADER_LOCAL_SERVER_H
#define YACREADER_LOCAL_SERVER_H

#include "yacreader_global.h"

#include <QMutex>
#include <QObject>
#include <QThread>

class QLocalServer;
class QLocalSocket;
class ComicDB;

class YACReaderLocalServer : public QObject
{
    Q_OBJECT
public:
    explicit YACReaderLocalServer(QObject *parent = nullptr);

signals:
    void comicUpdated(quint64 libraryId, const ComicDB &comic);
public slots:
    bool isListening();
    void sendResponse();
    static bool isRunning();
    void close();

private:
    // void run();
    QLocalServer *localServer;
};

class YACReaderClientConnectionWorker : public QThread
{
    Q_OBJECT
public:
    YACReaderClientConnectionWorker(QLocalSocket *clientConnection);
    ~YACReaderClientConnectionWorker() override;
signals:
    void comicUpdated(quint64 libraryId, const ComicDB &comic);

private:
    static QMutex dbMutex;
    // static int count;
    void run() override;

    // position: the index of the opened comic in siblings, -1 if it is not there
    void getComicInfo(quint64 libraryId, ComicDB &comic, QList<ComicDB> &siblings, qint32 &position);
    void getComicInfoFromReadingList(quint64 libraryId, const YACReader::OpenComicSource &source, ComicDB &comic, QList<ComicDB> &siblings, qint32 &position);
    void updateComic(quint64 libraryId, ComicDB &comic);
    void updateComic(quint64 libraryId, ComicDB &comic, qulonglong nextComicId);

    QLocalSocket *clientConnection;
};

#endif // YACREADER_LOCAL_SERVER_H
