#ifndef YACREADER_LOCAL_SERVER_H
#define YACREADER_LOCAL_SERVER_H

#include <QObject>

class QLocalServer;
class ComicDB;

class YACReaderLocalServer : public QObject
{
    Q_OBJECT
public:
    explicit YACReaderLocalServer(QObject *parent = 0);
    
signals:
    void comicUpdated(quint64 libraryId, const ComicDB & comic);
public slots:
    bool isListening();
    void sendResponse();
	static bool isRunning();
	void getComicInfo(quint64 libraryId, ComicDB & comic, QList<ComicDB> & sibling);
	void updateComic(quint64 libraryId, ComicDB & comic);
private:
    QLocalServer * localServer;
    
};

#endif // YACREADER_LOCAL_SERVER_H
