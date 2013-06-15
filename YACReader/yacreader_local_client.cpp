#include "yacreader_local_client.h"
#include "comic_db.h"

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

void YACReaderLocalClient::requestComicInfo(QString library, ComicDB & comic)
{

}

void YACReaderLocalClient::sendComicInfo(QString library, ComicDB & comic)
{

}
