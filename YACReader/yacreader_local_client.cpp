#include "yacreader_local_client.h"
#include "comic_db.h"
#include "yacreader_global.h"

#include <QLocalSocket>

YACReaderLocalClient::YACReaderLocalClient(QObject *parent) :
    QObject(parent)
{
    localSocket = new QLocalSocket(this);

    connect(localSocket, SIGNAL(readyRead()), this, SLOT(readMessage()));
	
    /*connect(socket, SIGNAL(error(QLocalSocket::LocalSocketError)),
            this, SLOT(displayError(QLocalSocket::LocalSocketError)));*/
}

//información de comic recibida...
void YACReaderLocalClient::readMessage()
{

}

bool YACReaderLocalClient::requestComicInfo(quint64 libraryId, ComicDB & comic)
{
	localSocket->connectToServer(YACREADERLIBRARY_GUID);
	if(localSocket->isOpen())
	{
		QByteArray block;
		QDataStream out(&block, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_0);
		out << (quint16)0;
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
		localSocket->waitForReadyRead();
		QByteArray data;
		QDataStream in(data);
		in.setVersion(QDataStream::Qt_4_0);
		int totalSize = 0;
		while((data.size()-sizeof(quint16)) != totalSize )
		{
			data.append(localSocket->read(1000000000));
			if(data.size()>=sizeof(quint16) && totalSize == 0)
				in >> totalSize;
		}
		in >> comic;
		return true;
	}
	else
		return false;
}

bool YACReaderLocalClient::sendComicInfo(quint64 libraryId, ComicDB & comic)
{
	return true;
}
