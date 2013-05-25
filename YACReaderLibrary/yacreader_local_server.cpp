#include "yacreader_local_server.h"

#include <QLocalServer>
#include <QLocalSocket>

#include "yacreader_global.h"

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

	     QByteArray block;
     QDataStream out(&block, QIODevice::WriteOnly);
     out.setVersion(QDataStream::Qt_4_0);
     out << (quint16)0;
     out << QString("ok");
     out.device()->seek(0);
     out << (quint16)(block.size() - sizeof(quint16));

     QLocalSocket *clientConnection = localServer->nextPendingConnection();
     connect(clientConnection, SIGNAL(disconnected()),
             clientConnection, SLOT(deleteLater()));

     clientConnection->write(block);
     clientConnection->flush();
     clientConnection->disconnectFromServer();

}

bool YACReaderLocalServer::isRunning()
{
	QLocalSocket socket;
	socket.connectToServer(YACREADERLIBRARY_GUID);
	if (socket.waitForConnected(500)) 
		return true; // Server is running (another instance of YACReaderLibrary has been launched)
	return false;
}