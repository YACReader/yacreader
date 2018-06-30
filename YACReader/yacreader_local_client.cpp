#include "yacreader_local_client.h"
#include "comic_db.h"
#include "yacreader_global.h"

#include <QLocalSocket>

#include "QsLog.h"

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
		out << (quint32)0;
		out << (quint8)YACReader::RequestComicInfo;
		out << libraryId;
		out << comic;
		out.device()->seek(0);
		out << (quint32)(block.size() - sizeof(quint32));

        int  written = 0;
        int previousWritten = 0;
		quint16 tries = 0;
		while(written != block.size() && tries < 200)
		{
			written += localSocket->write(block);
			localSocket->flush();
            if(written == previousWritten) //no bytes were written
                tries++;
            previousWritten = written;
		}
		if(tries == 200)
		{
			localSocket->close();
			QLOG_ERROR() << "Requesting Comic Info : unable to send request";
			return false;
		}
		
        localSocket->waitForBytesWritten(2000);

		//QByteArray data;
		tries = 0;
		int dataAvailable = 0;
		QByteArray packageSize;
        localSocket->waitForReadyRead(1000);
		while(packageSize.size() < sizeof(quint32) && tries < 20)
		{
			packageSize.append(localSocket->read(sizeof(quint32) - packageSize.size()));
			localSocket->waitForReadyRead(100);
			if(dataAvailable == packageSize.size())
			{
				tries++; //TODO apply 'tries' fix
			}
			dataAvailable = packageSize.size();
		}
		if(tries == 20)
		{
			localSocket->close();
			QLOG_ERROR() << "Requesting Comic Info : unable to read package size";
			return false;
		}
		QDataStream sizeStream(packageSize);//localSocket->read(sizeof(quint32)));
		sizeStream.setVersion(QDataStream::Qt_4_8);
		quint32 totalSize = 0;
		sizeStream >> totalSize;

		QByteArray data;

		tries = 0;
        int dataRead = 0;
		localSocket->waitForReadyRead(1000);
        while((unsigned int)data.length() < totalSize && tries < 20 )
		{
			data.append(localSocket->readAll());
            if((unsigned int)data.length() < totalSize)
				localSocket->waitForReadyRead(100);
            if(data.length() == dataRead)
                tries++;
            dataRead = data.length();
		}

		if(tries == 20)
		{
			localSocket->close();
			QLOG_ERROR() << "Requesting Comic Info : unable to read data (" << data.length() << "," << totalSize << ")";
			return false;
		}

		QDataStream dataStream(data);
		dataStream >> comic;
		dataStream >> siblings;
		localSocket->close();
		return true;
	}
	else
	{
		QLOG_ERROR() << "Requesting Comic Info : unable to connect to the server";
		return false;
	}
}


bool YACReaderLocalClient::sendComicInfo(quint64 libraryId, ComicDB & comic)
{
    localSocket->connectToServer(YACREADERLIBRARY_GUID);
    if(localSocket->isOpen())
    {
        //QLOG_INFO() << "Connection opened for sending ComicInfo";
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_8);
        out << (quint32)0;
        out << (quint8)YACReader::SendComicInfo;
        out << libraryId;
        out << comic;
        out.device()->seek(0);
        out << (quint32)(block.size() - sizeof(quint32));

        int  written, previousWritten;
        written = previousWritten = 0;
        int tries = 0;
        while(written != block.size() && tries < 100)
        {
            written += localSocket->write(block);
            if(written == previousWritten)
                tries++;
            previousWritten = written;
        }
        localSocket->waitForBytesWritten(2000);
        localSocket->close();
        //QLOG_INFO() << QString("Sending Comic Info : writen data (%1,%2)").arg(written).arg(block.size());
        if(tries == 100 && written != block.size())
        {
            emit finished();
            QLOG_ERROR() << QString("Sending Comic Info : unable to write data (%1,%2)").arg(written).arg(block.size());
            return false;
        }
        emit finished();
        return true;
    }

    emit finished();
    QLOG_ERROR() << "Sending Comic Info : unable to connect to the server";
    return false;
}

bool YACReaderLocalClient::sendComicInfo(quint64 libraryId, ComicDB & comic, qulonglong nextComicId)
{
    localSocket->connectToServer(YACREADERLIBRARY_GUID);
    if(localSocket->isOpen())
    {
        //QLOG_INFO() << "Connection opened for sending ComicInfo";
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_8);
        out << (quint32)0;
        out << (quint8)YACReader::SendComicInfo;
        out << libraryId;
        out << comic;
        out << nextComicId;
        out.device()->seek(0);
        out << (quint32)(block.size() - sizeof(quint32));

        int  written, previousWritten;
        written = previousWritten = 0;
        int tries = 0;
        while(written != block.size() && tries < 100)
        {
            written += localSocket->write(block);
            if(written == previousWritten)
                tries++;
            previousWritten = written;
        }
        localSocket->waitForBytesWritten(2000);
        localSocket->close();
        //QLOG_INFO() << QString("Sending Comic Info : writen data (%1,%2)").arg(written).arg(block.size());
        if(tries == 100 && written != block.size())
        {
            emit finished();
            QLOG_ERROR() << QString("Sending Comic Info : unable to write data (%1,%2)").arg(written).arg(block.size());
            return false;
        }
        emit finished();
        return true;
    }

    emit finished();
    QLOG_ERROR() << "Sending Comic Info : unable to connect to the server";
    return false;
}
