#ifndef YACREADER_LOCAL_SERVER_H
#define YACREADER_LOCAL_SERVER_H

#include <QObject>
#include <QThread>

class QLocalServer;
class QLocalSocket;
class ComicDB;

class YACReaderLocalServer : public QThread
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
private:
	void run();
	QLocalServer * localServer;
	
};

class YACReaderClientConnectionWorker : public QThread
{
	Q_OBJECT
public:
	YACReaderClientConnectionWorker( QLocalSocket *clientConnection);
	~YACReaderClientConnectionWorker();
signals:
	void comicUpdated(quint64 libraryId, const ComicDB & comic);
private:
	void run();
	
	void getComicInfo(quint64 libraryId, ComicDB & comic, QList<ComicDB> & sibling);
	void updateComic(quint64 libraryId, ComicDB & comic);

	QLocalSocket *clientConnection;
};

#endif // YACREADER_LOCAL_SERVER_H
