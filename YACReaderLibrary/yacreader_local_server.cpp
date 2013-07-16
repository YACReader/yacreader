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
	/*QLocalSocket *clientConnection = localServer->nextPendingConnection();

    connect(clientConnection, SIGNAL(disconnected()),
            clientConnection, SLOT(deleteLater()));

    QDataStream in(clientConnection);
    in.setVersion(QDataStream::Qt_4_0);

    if (clientConnection->bytesAvailable() == 0)
		return;
 
    if (in.atEnd())
        return;

    QString message;
    in >> message;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out << QString("OK");

    clientConnection->write(block);
    clientConnection->flush();
    clientConnection->disconnectFromServer();*/

     QLocalSocket *clientConnection = localServer->nextPendingConnection();
     connect(clientConnection, SIGNAL(disconnected()),
             clientConnection, SLOT(deleteLater()));

	 quint64 libraryId;
	 ComicDB comic;
	 clientConnection->waitForReadyRead();
	 QByteArray data;
	 QDataStream inputStream(data);
	 inputStream.setVersion(QDataStream::Qt_4_0);
	 int totalSize = 0;
	 while((data.size()-sizeof(quint16)) != totalSize )
	 {
		 data.append(clientConnection->read(1000000000));
		 if(data.size()>=sizeof(quint16) && totalSize == 0)
			 inputStream >> totalSize;
	 }
	 inputStream >> libraryId;
	 inputStream >> comic;

	 getComicInfo(libraryId,comic);

	 QByteArray block;
     QDataStream out(&block, QIODevice::WriteOnly);
     out.setVersion(QDataStream::Qt_4_0);
	 out << (quint16)0;
     out << comic;
	 out.device()->seek(0);
	 out << (quint16)(block.size() - sizeof(quint16));

	int  written = 0;
	while(written != block.size())
	{
		written += clientConnection->write(block);
		clientConnection->flush();
	}
	 //clientConnection->waitForBytesWritten();
     clientConnection->disconnectFromServer();

}

void YACReaderLocalServer::getComicInfo(quint64 libraryId, ComicDB & comic)
{
	comic = DBHelper::getComicInfo(DBHelper::getLibrariesNames().at(libraryId), comic.id);
}

bool YACReaderLocalServer::isRunning()
{
	QLocalSocket socket;
	socket.connectToServer(YACREADERLIBRARY_GUID);
	if (socket.waitForConnected(500)) 
		return true; // Server is running (another instance of YACReaderLibrary has been launched)
	return false;
}