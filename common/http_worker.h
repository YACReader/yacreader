#ifndef __HTTP_WORKER_H
#define __HTTP_WORKER_H

#include <QWidget>
#include <QByteArray>
#include <QThread>
#include <QUrl>
#include "yacreader_global.h"

 class HttpWorker : public QThread
 {
		 Q_OBJECT
	public:
		HttpWorker(const QString & urlString);
	public slots:
		void get();
		QByteArray getResult();
	private:
		void run();
		QUrl url;
		int httpGetId;
		QByteArray result;
	signals:
		void dataReady(const QByteArray &);
		void timeout();
 };

#endif
