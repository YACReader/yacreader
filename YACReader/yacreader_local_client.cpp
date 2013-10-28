#include "yacreader_local_client.h"
#include "comic_db.h"
#include "yacreader_global.h"

#include <QLocalSocket>

using namespace YACReader;

YACReaderLocalClient::YACReaderLocalClient(QObject *parent) :
	QObject(parent)
{
	localSocket = new QLocalSocket(this);

	//connect(localSocket, SIGNAL(readyRead()), this, SLOT(readMessage()));
	
	/*connect(socket, SIGNAL(error(QLocalSocket::LocalSocketError)),
			this, SLOT(displayError(QLocalSocket::LocalSocketError)));*/
}
YACReaderLocalClient::~YACReaderLocalClient()
{
	delete localSocket;
}
//información de comic recibida...
void YACReaderLocalClient::readMessage()
{

}
#include <QMessageBox>

bool YACReaderLocalClient::requestComicInfo(quint64 libraryId, ComicDB & comic, QList<ComicDB> & siblings)
{
	localSocket->connectToServer(YACREADERLIBRARY_GUID);
	if(localSocket->isOpen())
	{
		QByteArray block;
		QDataStream out(&block, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_8);
		out << (quint16)0;
		out << (quint8)YACReader::RequestComicInfo;
		out << libraryId;
		out << comic;
		out.device()->seek(0);
		out << (quint16)(block.size() - sizeof(quint16));

		int  written = 0;
		int tries = 0;
		while(written != block.size() && tries < 200)
		{
			written += localSocket->write(block);
			localSocket->flush();
			tries++;
		}
		if(tries == 200)
		{
			localSocket->close();
			return false;
		}
		
		//QByteArray data;
		tries = 0;
		while(localSocket->bytesAvailable() < sizeof(quint16) && tries < 10)
		{
			localSocket->waitForReadyRead(100);
			tries++;
		}
		if(tries == 10)
		{
			localSocket->close();
			return false;
		}
		QDataStream sizeStream(localSocket->read(sizeof(quint16)));
		sizeStream.setVersion(QDataStream::Qt_4_8);
		quint16 totalSize = 0;
		sizeStream >> totalSize;
		
		QByteArray data;

		tries = 0;
		while(data.length() < totalSize && tries < 10 )
		{
			data.append(localSocket->readAll());
			if(data.length() < totalSize)
				localSocket->waitForReadyRead(100);
			tries++;
		}
		if(tries == 10)
		{
			localSocket->close();
			return false;
		}

		QDataStream dataStream(data);
		dataStream >> comic;
		dataStream >> siblings;
		localSocket->close();
		return true;
	}
	else
		return false;
}

bool YACReaderLocalClient::sendComicInfo(quint64 libraryId, ComicDB & comic)
{
	localSocket->connectToServer(YACREADERLIBRARY_GUID);
	if(localSocket->isOpen())
	{
		QByteArray block;
		QDataStream out(&block, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_8);
		out << (quint16)0;
		out << (quint8)YACReader::SendComicInfo;
		out << libraryId;
		out << comic;
		out.device()->seek(0);
		out << (quint16)(block.size() - sizeof(quint16));

		int  written = 0;
		int tries = 0;
		while(written != block.size() && tries < 100)
		{
			written += localSocket->write(block);
			tries++;
		}
		localSocket->close();
		if(tries == 100 && written != block.size())
		{
			emit finished();
			return false;
		}
		emit finished();
		return true;
	}

	emit finished();
	return false;

}
