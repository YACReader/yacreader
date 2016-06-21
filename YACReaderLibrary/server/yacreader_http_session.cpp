#include "yacreader_http_session.h"

YACReaderHttpSession::YACReaderHttpSession(QObject *parent)
    : QObject(parent), comic(nullptr), remoteComic(nullptr), comicId(0), remoteComicId(0)
{

}

bool YACReaderHttpSession::isComicOnDevice(const QString & hash)
{
    return comicsOnDevice.contains(hash);
}

bool YACReaderHttpSession::isComicDownloaded(const QString & hash)
{
    return downloadedComics.contains(hash);
}

void YACReaderHttpSession::setComicOnDevice(const QString & hash)
{
    comicsOnDevice.insert(hash);
}

void YACReaderHttpSession::setComicsOnDevice(const QSet<QString> & set)
{
    comicsOnDevice = set;
}

void YACReaderHttpSession::setDownloadedComic(const QString & hash)
{
    downloadedComics.insert(hash);
}

QSet<QString> YACReaderHttpSession::getComicsOnDevice()
{
    return comicsOnDevice ;
}

QSet<QString> YACReaderHttpSession::getDownloadedComics()
{
    return downloadedComics ;
}

void YACReaderHttpSession::clearComics()
{
    comicsOnDevice.clear();
    downloadedComics.clear();
}
//current comic (import)
qulonglong YACReaderHttpSession::getCurrentComicId()
{
    return comicId;
}

Comic* YACReaderHttpSession::getCurrentComic()
{
    comic;
}

void YACReaderHttpSession::dismissCurrentComic()
{
    if(comic != nullptr)
    {
        comic->deleteLater();
        comic = nullptr;
    }
}

void YACReaderHttpSession::setCurrentComic(qulonglong id, Comic * comic)
{
    dismissCurrentComic();
    comicId = id;
    comic = comic;
}

//current comic (read)
qulonglong YACReaderHttpSession::getCurrentRemoteComicId()
{
    return remoteComicId ;
}

Comic* YACReaderHttpSession::getCurrentRemoteComic()
{
    return remoteComic ;
}

void YACReaderHttpSession::dismissCurrentRemoteComic()
{
    if(remoteComic != nullptr)
    {
        remoteComic->deleteLater();
        remoteComic = nullptr;
    }
}

void YACReaderHttpSession::setCurrentRemoteComic(qulonglong id, Comic * comic)
{
    dismissCurrentRemoteComic();
    remoteComicId = id;
    remoteComic = comic;
}

QString YACReaderHttpSession::getDeviceType()
{
    return device;
}

QString YACReaderHttpSession::getDisplayType()
{
    return display;
}

void YACReaderHttpSession::setDeviceType(const QString & device)
{
    //comicsOnDevice.clear(); //TODO crear un m�todo clear que limpie la sesi�n completamente
    //downloadedComics.clear();
    device = device;
}

void YACReaderHttpSession::setDisplayType(const QString & display)
{
    display = display;
}

void YACReaderHttpSession::clearNavigationPath()
{
    navigationPath.clear();
}

QPair<qulonglong, quint32> YACReaderHttpSession::popNavigationItem()
{
    if(navigationPath.isEmpty() == false)
        return navigationPath.pop();
    return QPair<qulonglong, quint32>();
}

QPair<qulonglong, quint32> YACReaderHttpSession::topNavigationItem()
{
    if(navigationPath.isEmpty() == false)
        return navigationPath.top();
    return QPair<qulonglong, quint32>();
}

void YACReaderHttpSession::pushNavigationItem(const QPair<qulonglong, quint32> &item)
{
    navigationPath.push(item);
}

void YACReaderHttpSession::updateTopItem(const QPair<qulonglong, quint32> &item)
{
     if(navigationPath.isEmpty() == false)
     {
        navigationPath.pop();
        navigationPath.push(item);
     }
     else
     {
         navigationPath.push(item);
     }
}

QStack<QPair<qulonglong, quint32> > YACReaderHttpSession::getNavigationPath()
{
    return navigationPath;
}
