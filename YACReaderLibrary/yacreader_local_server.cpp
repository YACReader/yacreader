#include "yacreader_local_server.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QPointer>

#include "yacreader_global.h"
#include "db_helper.h"

#include "comic_db.h"

using namespace YACReader;

QMutex YACReaderClientConnectionWorker::dbMutex;

YACReaderLocalServer::YACReaderLocalServer(QObject *parent) :
	QThread(parent)
{
	localServer = new QLocalServer(this);
	if (!localServer->listen(YACREADERLIBRARY_GUID)) {
		//error...........
	}

	connect(localServer, SIGNAL(newConnection()), this, SLOT(sendResponse()));
}

bool YACReaderLocalServer::isListening()
{
	return localServer->isListening();
}

void YACReaderLocalServer::run()
{
	while(1)
		exec();
}

void YACReaderLocalServer::sendResponse()
{
	QLocalSocket *clientConnection = localServer->nextPendingConnection();
	//connect(clientConnection, SIGNAL(disconnected()),clientConnection, SLOT(deleteLater()));

	YACReaderClientConnectionWorker * worker = new YACReaderClientConnectionWorker(clientConnection);
	connect(worker,SIGNAL(comicUpdated(quint64, ComicDB)),this,SIGNAL(comicUpdated(quint64, ComicDB)));
	connect(worker,SIGNAL(finished()),worker,SLOT(deleteLater()));
	worker->start();
	//clientConnection->waitForBytesWritten();*/
	//clientConnection->disconnectFromServer();
}

bool YACReaderLocalServer::isRunning()
{
	QLocalSocket socket;
	socket.connectToServer(YACREADERLIBRARY_GUID);
	if (socket.waitForConnected(500)) 
		return true; // Server is running (another instance of YACReaderLibrary has been launched)
	return false;
}


YACReaderClientConnectionWorker::YACReaderClientConnectionWorker( QLocalSocket *cc)
	:QThread(),clientConnection(cc)
{

}

YACReaderClientConnectionWorker::~YACReaderClientConnectionWorker()
{

}

void YACReaderClientConnectionWorker::run()
{
	quint64 libraryId;
	ComicDB comic;
	int tries = 0;
	//QByteArray data;
	while(clientConnection->bytesAvailable() < sizeof(quint16) && tries < 200)
	{
		clientConnection->waitForReadyRead(10);
		tries++;
	}
	if(tries == 200)
		return;
	QDataStream sizeStream(clientConnection->read(sizeof(quint16)));
	sizeStream.setVersion(QDataStream::Qt_4_8);
	quint16 totalSize = 0;
	sizeStream >> totalSize;

	tries = 0;
	while(clientConnection->bytesAvailable() < totalSize && tries < 200)
	{
		clientConnection->waitForReadyRead(10);
		tries++;
	}
	if(tries == 200)
		return;
	QDataStream dataStream(clientConnection->read(totalSize));
	quint8 msgType;
	dataStream >> msgType;
	dataStream >> libraryId;
	dataStream >> comic;

	switch (msgType)
	{
	case YACReader::RequestComicInfo:
		{
			QList<ComicDB> siblings;
			getComicInfo(libraryId,comic,siblings);

			QByteArray block;
			QDataStream out(&block, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_8);
			out << (quint16)0;
			out << comic;
			out << siblings;
			out.device()->seek(0);
			out << (quint16)(block.size() - sizeof(quint16));

			int  written = 0;
			tries = 0;
			while(written != block.size() && tries < 200)
			{
				int ret = clientConnection->write(block);
				if(ret != -1)
				{
					written += ret;
					clientConnection->flush();
				}
				else
					tries++;
			}
			break;
		}
	case YACReader::SendComicInfo:
		{
			updateComic(libraryId,comic);
			//clientConnection->disconnectFromServer();
			break;
		}

	}

	clientConnection->waitForDisconnected();
	clientConnection->deleteLater();
}

void YACReaderClientConnectionWorker::getComicInfo(quint64 libraryId, ComicDB & comic, QList<ComicDB> & siblings)
{
	QMutexLocker locker(&dbMutex);
	comic = DBHelper::getComicInfo(DBHelper::getLibrariesNames().at(libraryId), comic.id);
	siblings = DBHelper::getSiblings(DBHelper::getLibrariesNames().at(libraryId), comic.parentId);
}

void YACReaderClientConnectionWorker::updateComic(quint64 libraryId, ComicDB & comic)
{
	QMutexLocker locker(&dbMutex);
	DBHelper::update(DBHelper::getLibrariesNames().at(libraryId), comic.info);
	emit comicUpdated(libraryId, comic);
}
