#include "check_new_version.h"
#include <QMessageBox>
#include <QUrl>
#include <QtGlobal>
#include <QStringList>

#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QTimer>
#include <QNetworkRequest>
#include <QNetworkReply>

#define PREVIOUS_VERSION "6.0.0"

HttpVersionChecker::HttpVersionChecker()
		:QThread()
{
	http = new QHttp(this);

	connect(http, SIGNAL(requestFinished(int, bool)),
			 this, SLOT(httpRequestFinished(int, bool)));

	connect(http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)),
			 this, SLOT(readResponseHeader(const QHttpResponseHeader &)));

	connect(http, SIGNAL(readyRead(const QHttpResponseHeader &)),
			this, SLOT(read(const QHttpResponseHeader &)));
}

void HttpVersionChecker::get()
{
	this->start();

}

void HttpVersionChecker::run()
{
	QNetworkAccessManager manager;
	QEventLoop q;
	QTimer tT;
	
	tT.setSingleShot(true);
	connect(&tT, SIGNAL(timeout()), &q, SLOT(quit()));
	connect(&manager, SIGNAL(finished(QNetworkReply*)),&q, SLOT(quit()));
	QNetworkReply *reply = manager.get(QNetworkRequest(
				   QUrl("https://bitbucket.org/luisangelsm/yacreader/wiki/Home")));
	
	tT.start(5000); // 5s timeout
	q.exec();
	
	if(tT.isActive()){
		// download complete
		checkNewVersion(reply->readAll());
		tT.stop();
	} else {
		// timeout
	}
	
	/*QUrl url("http://code.google.com/p/yacreader/downloads/list");
	QHttp::ConnectionMode mode = QHttp::ConnectionModeHttp;
	http->setHost(url.host(), mode, url.port() == -1 ? 0 : url.port());
	QByteArray path = QUrl::toPercentEncoding(url.path(), "!$&'()*+,;=:@/");
	if (path.isEmpty())
		 path = "/";
	httpGetId = http->get(path, 0);
	exec();*/
}
void HttpVersionChecker::readResponseHeader(const QHttpResponseHeader &responseHeader)
{

}

void HttpVersionChecker::read(const QHttpResponseHeader &){
	content.append(http->readAll());
}

void HttpVersionChecker::httpRequestFinished(int requestId, bool error)
{
#ifdef QT_DEBUG
	QString response("YACReader-5.0.0 win32.exe");
#else
	QString response(content);
#endif
	checkNewVersion(response);
	exit();
}

//TODO escribir prueba unitaria
bool HttpVersionChecker::checkNewVersion(QString sourceContent)
{
#ifdef Q_OS_WIN32
	QRegExp rx(".*YACReader\\-([0-9]+).([0-9]+).([0-9]+)\\.?([0-9]+)?.{0,5}win32.*");
#endif

#ifdef  Q_OS_LINUX
	QRegExp rx(".*YACReader\\-([0-9]+).([0-9]+).([0-9]+)\\.?([0-9]+)?.{0,5}X11.*");
#endif

#ifdef  Q_OS_MAC
	QRegExp rx(".*YACReader\\-([0-9]+).([0-9]+).([0-9]+)\\.?([0-9]+)?.{0,5}Mac.*");
#endif
	
	int index = 0;
	bool newVersion = false;
	bool sameVersion = true;
	//bool currentVersionIsNewer = false;
#ifdef QT_DEBUG
	QString version(PREVIOUS_VERSION);
#else
	QString version(VERSION);
#endif
	QStringList sl = version.split(".");
	if((index = rx.indexIn(sourceContent))!=-1)
	{
		int length = qMin(sl.size(),(rx.cap(4)!="")?4:3);
		for(int i=0;i<length;i++)
		{
			if(rx.cap(i+1).toInt()<sl.at(i).toInt())
			{
				return false;
			}
			if(rx.cap(i+1).toInt()>sl.at(i).toInt()){
				newVersion=true;
				break;
			}
			else
				sameVersion = sameVersion && rx.cap(i+1).toInt()==sl.at(i).toInt();
		}
		if(!newVersion && sameVersion)
		{
			if((sl.size()==3)&&(rx.cap(4)!=""))
				newVersion = true;
		}


	}

	if(newVersion == true)
	{
		emit newVersionDetected();
		return true;
	}
	else
	{
		return false;
	}
}