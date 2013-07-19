#include "yacreader_local_client.h"
#include "comic_db.h"
#include "yacreader_global.h"

#include <QLocalSocket>

YACReaderLocalClient::YACReaderLocalClient(QObject *parent) :
    QObject(parent)
{
    localSocket = new QLocalSocket(this);

    //connect(localSocket, SIGNAL(readyRead()), this, SLOT(readMessage()));
	
    /*connect(socket, SIGNAL(error(QLocalSocket::LocalSocketError)),
            this, SLOT(displayError(QLocalSocket::LocalSocketError)));*/
}

//información de comic recibida...
void YACReaderLocalClient::readMessage()
{

}
#include <QFile>
bool YACReaderLocalClient::requestComicInfo(quint64 libraryId, ComicDB & comic, QList<ComicDB> & siblings)
{
	localSocket->connectToServer(YACREADERLIBRARY_GUID);
	if(localSocket->isOpen())
	{
			 	 QFile f("c:/temp/socket.txt");
	 f.open(QIODevice::WriteOnly);
	 QTextStream outt(&f);
	  outt << QString(" antes : %1").arg(comic.id) << endl;

		QByteArray block;
		QDataStream out(&block, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_8);
		out << (quint16)0;
		out << (quint8)YACReaderIPCMessages::RequestComicInfo;
		out << libraryId;
		out << comic;
		out.device()->seek(0);
		out << (quint16)(block.size() - sizeof(quint16));

		int  written = 0;
		while(written != block.size())
		{
			written += localSocket->write(block);
			localSocket->flush();
		}
		
		//QByteArray data;
		while(localSocket->bytesAvailable() < sizeof(quint16))
			localSocket->waitForReadyRead();
		QDataStream sizeStream(localSocket->read(sizeof(quint16)));
		sizeStream.setVersion(QDataStream::Qt_4_8);
		quint16 totalSize = 0;
		sizeStream >> totalSize;
		
		while(localSocket->bytesAvailable() < totalSize )
		{
			localSocket->waitForReadyRead();
		}
		QDataStream dataStream(localSocket->read(totalSize));
		dataStream >> comic;
		dataStream >> siblings;
			 outt << QString(" despues : %1").arg(comic.id) << endl;
		return true;
	}
	else
		return false;
}

bool YACReaderLocalClient::sendComicInfo(quint64 libraryId, ComicDB & comic)
{
	return true;
}
