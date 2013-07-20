#include "yacreader_local_server.h"

#include <QLocalServer>
#include <QLocalSocket>

#include "yacreader_global.h"
#include "db_helper.h"

#include "comic_db.h"

YACReaderLocalServer::YACReaderLocalServer(QObject *parent) :
    QObject(parent)
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

void YACReaderLocalServer::sendResponse()
{
	 QLocalSocket *clientConnection = localServer->nextPendingConnection();
     connect(clientConnection, SIGNAL(disconnected()),
             clientConnection, SLOT(deleteLater()));

	 quint64 libraryId;
	 ComicDB comic;

	 //QByteArray data;
	 while(clientConnection->bytesAvailable() < sizeof(quint16))
		 clientConnection->waitForReadyRead();
	 QDataStream sizeStream(clientConnection->read(sizeof(quint16)));
	 sizeStream.setVersion(QDataStream::Qt_4_8);
	 quint16 totalSize = 0;
	 sizeStream >> totalSize;

	 while(clientConnection->bytesAvailable() < totalSize )
	 {
		 clientConnection->waitForReadyRead();
	 }
	 QDataStream dataStream(clientConnection->read(totalSize));
	 quint8 msgType;
	 dataStream >> msgType;
	 dataStream >> libraryId;
	 dataStream >> comic;

	 switch (msgType)
	 {
	 case YACReaderIPCMessages::RequestComicInfo:
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
			 while(written != block.size())
			 {
				 written += clientConnection->write(block);
				 clientConnection->flush();
			 }
			 break;
		 }
	 case YACReaderIPCMessages::SendComicInfo:
		 {
			 updateComic(libraryId,comic);
			 //clientConnection->disconnectFromServer();
			 break;
		 }

	 }

	 //clientConnection->waitForBytesWritten();*/
	 //clientConnection->disconnectFromServer();
}

void YACReaderLocalServer::getComicInfo(quint64 libraryId, ComicDB & comic, QList<ComicDB> & siblings)
{
	comic = DBHelper::getComicInfo(DBHelper::getLibrariesNames().at(libraryId), comic.id);
	siblings = DBHelper::getSiblings(DBHelper::getLibrariesNames().at(libraryId), comic.parentId);
}

void YACReaderLocalServer::updateComic(quint64 libraryId, ComicDB & comic)
{
	DBHelper::update(DBHelper::getLibrariesNames().at(libraryId), comic.info);
}

bool YACReaderLocalServer::isRunning()
{
	QLocalSocket socket;
	socket.connectToServer(YACREADERLIBRARY_GUID);
	if (socket.waitForConnected(500)) 
		return true; // Server is running (another instance of YACReaderLibrary has been launched)
	return false;
}