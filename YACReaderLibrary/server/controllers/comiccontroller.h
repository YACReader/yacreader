#ifndef COMICCONTROLLER_H
#define COMICCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

#include <QThread>
class Comic2;
class QString;

class ComicController : public HttpRequestHandler {
	Q_OBJECT
	Q_DISABLE_COPY(ComicController);
public:
	/** Constructor */
	ComicController();

	/** Generates the response */
	void service(HttpRequest& request, HttpResponse& response);
};

class Synchronizer : public QThread
{

	Q_OBJECT
public:
		Synchronizer(Comic2 * c, QString p, QThread * f);
	bool loaded;
private:
	Comic2 * comic;
	QString path;
	QThread * from;
	
	void run();
	public slots:
		void waitedSignal();
};

#endif // COMICCONTROLLER_H
