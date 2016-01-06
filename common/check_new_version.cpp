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

#define PREVIOUS_VERSION_TESTING "6.0.0"

HttpVersionChecker::HttpVersionChecker()
    :HttpWorker("https://bitbucket.org/luisangelsm/yacreader/wiki/Home")
{
	connect(this,SIGNAL(dataReady(const QByteArray &)),this,SLOT(checkNewVersion(const QByteArray &)));
}

void HttpVersionChecker::checkNewVersion(const QByteArray & data)
{
	checkNewVersion(QString(data));
}

bool HttpVersionChecker::checkNewVersion(QString sourceContent)
{
#ifdef Q_OS_WIN32
    QRegExp rx("YACReader\\-([0-9]+).([0-9]+).([0-9]+)\\.?([0-9]+)?.{0,5}win32");
#endif

#if defined  Q_OS_UNIX && !defined Q_OS_MAC
    QRegExp rx("YACReader\\-([0-9]+).([0-9]+).([0-9]+)\\.?([0-9]+)?.{0,5}X11");
#endif

#ifdef  Q_OS_MAC
    QRegExp rx("YACReader\\-([0-9]+).([0-9]+).([0-9]+)\\.?([0-9]+)?.{0,5}Mac");
#endif
	
	int index = 0;
	bool newVersion = false;
	bool sameVersion = true;
	//bool currentVersionIsNewer = false;
#ifdef QT_DEBUG
    QString version(PREVIOUS_VERSION_TESTING);
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
