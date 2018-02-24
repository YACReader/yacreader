/**
  @file
  @author Stefan Frings
*/

#include "httpsession.h"
#include <QDateTime>
#include <QUuid>


HttpSession::HttpSession(bool canStore) {
	if (canStore) {
		dataPtr=new HttpSessionData();
		dataPtr->refCount=1;
		dataPtr->lastAccess=QDateTime::currentMSecsSinceEpoch();
		dataPtr->id=QUuid::createUuid().toString().toLatin1();
		dataPtr->yacreaderSessionData.comic = 0;
		dataPtr->yacreaderSessionData.comicId = 0;
        dataPtr->yacreaderSessionData.remoteComic = 0;
        dataPtr->yacreaderSessionData.remoteComicId = 0;
#ifdef SUPERVERBOSE
		qDebug("HttpSession: created new session data with id %s",dataPtr->id.data());
#endif
	}
	else {
		dataPtr=0;
	}
}

HttpSession::HttpSession(const HttpSession& other) {
	dataPtr=other.dataPtr;
	if (dataPtr) {
		dataPtr->lock.lockForWrite();
		dataPtr->refCount++;
#ifdef SUPERVERBOSE
		qDebug("HttpSession: refCount of %s is %i",dataPtr->id.data(),dataPtr->refCount);
#endif
		dataPtr->lock.unlock();
	}
}

HttpSession& HttpSession::operator= (const HttpSession& other) {
	HttpSessionData* oldPtr=dataPtr;
	dataPtr=other.dataPtr;
	if (dataPtr) {
		dataPtr->lock.lockForWrite();
		dataPtr->refCount++;
#ifdef SUPERVERBOSE
		qDebug("HttpSession: refCount of %s is %i",dataPtr->id.data(),dataPtr->refCount);
#endif
		dataPtr->lastAccess=QDateTime::currentMSecsSinceEpoch();
		dataPtr->lock.unlock();
	}
	if (oldPtr) {
		int refCount;
		oldPtr->lock.lockForRead();
		refCount=oldPtr->refCount--;
#ifdef SUPERVERBOSE
		qDebug("HttpSession: refCount of %s is %i",oldPtr->id.data(),oldPtr->refCount);
#endif
		oldPtr->lock.unlock();
		if (refCount==0) {
			delete oldPtr;
		}
	}
	return *this;
}

HttpSession::~HttpSession() {
	if (dataPtr) {
		int refCount;
		dataPtr->lock.lockForRead();
		refCount=--dataPtr->refCount;
#ifdef SUPERVERBOSE
		qDebug("HttpSession: refCount of %s is %i",dataPtr->id.data(),dataPtr->refCount);
#endif
		dataPtr->lock.unlock();
		if (refCount==0) {
			qDebug("HttpSession: deleting data");
			delete dataPtr;
		}
	}
}


QByteArray HttpSession::getId() const {
	if (dataPtr) {
		return dataPtr->id;
	}
	else {
		return QByteArray();
	}
}

bool HttpSession::isNull() const {
	return dataPtr==0;
}

void HttpSession::set(const QByteArray& key, const QVariant& value) {
	if (dataPtr) {
		dataPtr->lock.lockForWrite();
		dataPtr->values.insert(key,value);
		dataPtr->lock.unlock();
	}
}

void HttpSession::remove(const QByteArray& key) {
	if (dataPtr) {
		dataPtr->lock.lockForWrite();
		dataPtr->values.remove(key);
		dataPtr->lock.unlock();
	}
}

QVariant HttpSession::get(const QByteArray& key) const {
	QVariant value;
	if (dataPtr) {
		dataPtr->lock.lockForRead();
		value=dataPtr->values.value(key);
		dataPtr->lock.unlock();
	}
	return value;
}

bool HttpSession::contains(const QByteArray& key) const {
	bool found=false;
	if (dataPtr) {
		dataPtr->lock.lockForRead();
		found=dataPtr->values.contains(key);
		dataPtr->lock.unlock();
	}
	return found;
}

QMap<QByteArray,QVariant> HttpSession::getAll() const {
	QMap<QByteArray,QVariant> values;
	if (dataPtr) {
		dataPtr->lock.lockForRead();
		values=dataPtr->values;
		dataPtr->lock.unlock();
	}
	return values;
}

qint64 HttpSession::getLastAccess() const {
	qint64 value=0;
	if (dataPtr) {
		dataPtr->lock.lockForRead();
		value=dataPtr->lastAccess;
		dataPtr->lock.unlock();
	}
	return value;
}


void HttpSession::setLastAccess() {
	if (dataPtr) {
		dataPtr->lock.lockForRead();
		dataPtr->lastAccess=QDateTime::currentMSecsSinceEpoch();
		dataPtr->lock.unlock();
	}
}

//AÑADIDO
//sets
bool HttpSession::isComicOnDevice(const QString & hash)
{
	if(dataPtr)
		return dataPtr->yacreaderSessionData.comicsOnDevice.contains(hash);
	else
		return false;
}
bool HttpSession::isComicDownloaded(const QString & hash)
{
	if(dataPtr)
		return dataPtr->yacreaderSessionData.downloadedComics.contains(hash);
	else
		return false;
}
void HttpSession::setComicOnDevice(const QString & hash)
{
	if(dataPtr)
	{
		dataPtr->yacreaderSessionData.comicsOnDevice.insert(hash);
	}
}
void HttpSession::setComicsOnDevice(const QSet<QString> & set)
{
	if(dataPtr)
	{
		dataPtr->yacreaderSessionData.comicsOnDevice = set;
	}
}
void HttpSession::setDownloadedComic(const QString & hash)
{
	if(dataPtr)
	{
		dataPtr->yacreaderSessionData.downloadedComics.insert(hash);
	}
}
QSet<QString> HttpSession::getComicsOnDevice()
{
	if(dataPtr)
		return dataPtr->yacreaderSessionData.comicsOnDevice ;
	else
		return QSet<QString>();
}
QSet<QString> HttpSession::getDownloadedComics()
{
	if(dataPtr)
		return dataPtr->yacreaderSessionData.downloadedComics ;
	else
        return QSet<QString>();
}

void HttpSession::clearComics()
{
    if(dataPtr)
    {
        dataPtr->yacreaderSessionData.comicsOnDevice.clear();
        dataPtr->yacreaderSessionData.downloadedComics.clear();
    }
}
//current comic (import)
qulonglong HttpSession::getCurrentComicId()
{
	if(dataPtr)
		return dataPtr->yacreaderSessionData.comicId ;
	else
		return 0;
}
Comic* HttpSession::getCurrentComic()
{
	if(dataPtr)
	{
		return dataPtr->yacreaderSessionData.comic ;
	}
	else
		return 0;
}
void HttpSession::dismissCurrentComic()
{
	if(dataPtr)
	{
		if(dataPtr->yacreaderSessionData.comic != 0)
		{
            dataPtr->yacreaderSessionData.comic->invalidate();
			dataPtr->yacreaderSessionData.comic->deleteLater();
			dataPtr->yacreaderSessionData.comic = 0;
		}
		dataPtr->yacreaderSessionData.comicId = 0;
	}
}
void HttpSession::setCurrentComic(qulonglong id, Comic * comic)
{
	if(dataPtr)
	{
		dismissCurrentComic();
		dataPtr->yacreaderSessionData.comicId = id;
		dataPtr->yacreaderSessionData.comic = comic;
	}
}

//current comic (read)
qulonglong HttpSession::getCurrentRemoteComicId()
{
    if(dataPtr)
        return dataPtr->yacreaderSessionData.remoteComicId ;
    else
        return 0;
}
Comic* HttpSession::getCurrentRemoteComic()
{
    if(dataPtr)
    {
        return dataPtr->yacreaderSessionData.remoteComic ;
    }
    else
        return 0;
}
void HttpSession::dismissCurrentRemoteComic()
{
    if(dataPtr)
    {
        if(dataPtr->yacreaderSessionData.remoteComic != 0)
        {
            dataPtr->yacreaderSessionData.remoteComic->invalidate();
            dataPtr->yacreaderSessionData.remoteComic->deleteLater();
            dataPtr->yacreaderSessionData.remoteComic = 0;
        }
        dataPtr->yacreaderSessionData.remoteComicId = 0;
    }
}
void HttpSession::setCurrentRemoteComic(qulonglong id, Comic * comic)
{
    if(dataPtr)
    {
        dismissCurrentRemoteComic();
        dataPtr->yacreaderSessionData.remoteComicId = id;
        dataPtr->yacreaderSessionData.remoteComic = comic;
    }
}


QString HttpSession::getDeviceType()
{
	if(dataPtr)
	{
		return dataPtr->yacreaderSessionData.device;
	}
	return "";
}
QString HttpSession::getDisplayType()
{
	if(dataPtr)
	{
		return dataPtr->yacreaderSessionData.display;
	}
	return "";
}
void HttpSession::setDeviceType(const QString & device)
{
	if(dataPtr)
	{
        //dataPtr->yacreaderSessionData.comicsOnDevice.clear(); //TODO crear un método clear que limpie la sesión completamente
        //dataPtr->yacreaderSessionData.downloadedComics.clear();
		dataPtr->yacreaderSessionData.device = device;
	}
}
void HttpSession::setDisplayType(const QString & display)
{
	if(dataPtr)
	{
		dataPtr->yacreaderSessionData.display = display;
	}
}

void HttpSession::clearNavigationPath()
{
    if(dataPtr)
        dataPtr->yacreaderSessionData.navigationPath.clear();
}

QPair<qulonglong, quint32> HttpSession::popNavigationItem()
{
    if(dataPtr && !(dataPtr->yacreaderSessionData.navigationPath.isEmpty()))
        return dataPtr->yacreaderSessionData.navigationPath.pop();
    return QPair<qulonglong, quint32>();
}

QPair<qulonglong, quint32> HttpSession::topNavigationItem()
{
    if(dataPtr && !(dataPtr->yacreaderSessionData.navigationPath.isEmpty()))
        return dataPtr->yacreaderSessionData.navigationPath.top();
    return QPair<qulonglong, quint32>();
}

void HttpSession::pushNavigationItem(const QPair<qulonglong, quint32> &item)
{
    if(dataPtr)
        dataPtr->yacreaderSessionData.navigationPath.push(item);
}

void HttpSession::updateTopItem(const QPair<qulonglong, quint32> &item)
{
     if(dataPtr && !(dataPtr->yacreaderSessionData.navigationPath.isEmpty()))
     {
        dataPtr->yacreaderSessionData.navigationPath.pop();
        dataPtr->yacreaderSessionData.navigationPath.push(item);
     } else if(dataPtr)
     {
         dataPtr->yacreaderSessionData.navigationPath.push(item);
     }
}

QStack<QPair<qulonglong, quint32> > HttpSession::getNavigationPath()
{
    if(dataPtr)
        return dataPtr->yacreaderSessionData.navigationPath;
    else
        return QStack<QPair<qulonglong, quint32> >();
}

