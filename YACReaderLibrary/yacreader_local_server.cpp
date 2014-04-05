#include "yacreader_local_server.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QPointer>

#include "yacreader_global.h"
#include "db_helper.h"

#include "comic_db.h"

#include "QsLog.h"

using namespace YACReader;

QMutex YACReaderClientConnectionWorker::dbMutex;
//int YACReaderClientConnectionWorker::count = 0;
YACReaderLocalServer::YACReaderLocalServer(QObject *parent) :
	QObject(parent)
{
	localServer = new QLocalServer(this);
    QLocalServer::removeServer(YACREADERLIBRARY_GUID);
	if (!localServer->listen(YACREADERLIBRARY_GUID)) {
        QLOG_ERROR() << "Unable to create local server";
	}

	connect(localServer, SIGNAL(newConnection()), this, SLOT(sendResponse()));
}

bool YACReaderLocalServer::isListening()
{
	return localServer->isListening();
}

/*void YACReaderLocalServer::run()
{
	while(1)
		exec();
}*/

void YACReaderLocalServer::sendResponse()
{
	QLocalSocket *clientConnection = localServer->nextPendingConnection();
	//connect(clientConnection, SIGNAL(disconnected()),clientConnection, SLOT(deleteLater()));
	clientConnection->setParent(0);

	YACReaderClientConnectionWorker * worker = new YACReaderClientConnectionWorker(clientConnection);
	if(worker != 0)
	{
		clientConnection->moveToThread(worker);
		connect(worker,SIGNAL(comicUpdated(quint64, ComicDB)),this,SIGNAL(comicUpdated(quint64, ComicDB)));
		connect(worker,SIGNAL(finished()),worker,SLOT(deleteLater()));
		worker->start();
	}

	QLOG_INFO() << "connection incoming";
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
/*#include <QFile>
#include <QTextStream>
#include <QDateTime>*/
void YACReaderClientConnectionWorker::run()
{
	/*{
	QFile f(QString("c:/temp/thread%1.txt").arg(count));
	f.open(QIODevice::Append);
	QTextStream out(&f);
	out << QString("Thread%1 starts").arg(count) << endl;
	f.close();
	}
	uint t1 = QDateTime::currentMSecsSinceEpoch();*/

	quint64 libraryId;
	ComicDB comic;
	int tries = 0;
	int dataAvailable = 0;
	QByteArray packageSize;
	clientConnection->waitForReadyRead(1000);
	while(packageSize.size() < sizeof(quint32) && tries < 20)
	{
		packageSize.append(clientConnection->read(sizeof(quint32) - packageSize.size()));
		clientConnection->waitForReadyRead(100);
		if(dataAvailable == packageSize.size())
		{
			tries++;
		}
		dataAvailable = packageSize.size();
	}
	if(tries == 20)
	{
		QLOG_ERROR() << "Local connection: unable to read the message size";
		return;
	}

	QDataStream sizeStream(packageSize);
	sizeStream.setVersion(QDataStream::Qt_4_8);
	quint32 totalSize = 0;
	sizeStream >> totalSize;

	tries = 0;
	QByteArray data;
    int dataRead = 0;
	while(data.size() < totalSize && tries < 200)
	{
		data.append(clientConnection->readAll());
		if(data.length() < totalSize)
			clientConnection->waitForReadyRead(100);
        if(dataRead == data.length()) //no bytes were read
            tries++;
        dataRead = data.length();
	}
	if(tries == 200)
	{
		QLOG_ERROR() << QString("Local connection: unable to read message (%1,%2)").arg(data.size()).arg(totalSize);
		return;
	}
	QDataStream dataStream(data);
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
			out << (quint32)0;
			out << comic;
			out << siblings;
			out.device()->seek(0);
			out << (quint32)(block.size() - sizeof(quint32));

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
			if(tries == 200 && written != block.size())
				QLOG_ERROR() << QString("Local connection (comic info requested): unable to send response (%1,%2)").arg(written).arg(block.size());
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
	/*count++;
	uint t2 = QDateTime::currentMSecsSinceEpoch();
	{
	QFile f(QString("c:/temp/thread%1.txt").arg(count));
	f.open(QIODevice::Append);
	QTextStream out(&f);
	out << QString("Thread%1 ends : time - %2").arg(count).arg(t2-t1) << endl;
	f.close();
	}*/
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
