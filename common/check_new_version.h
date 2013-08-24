#ifndef __CHECKUPDATE_H
#define __CHECKUPDATE_H

#include <QWidget>
#include <QHttp>
#include <QHttpResponseHeader>
#include <QByteArray>
#include <QThread>
#include "yacreader_global.h"

 class HttpVersionChecker : public QThread
 {
		 Q_OBJECT
	public:
		HttpVersionChecker();
		bool thereIsNewVersion();
	public slots:
		void httpRequestFinished(int requestId, bool error);
		void readResponseHeader(const QHttpResponseHeader &);
		void read(const QHttpResponseHeader &);
		void get();
	private:
		void run();
		QHttp *http;
		int httpGetId;
		QByteArray content;
		bool found;
		bool checkNewVersion(QString sourceContent);
	signals:
		void newVersionDetected();
 };

#endif
