#include "pagecontroller.h"

#include "../static.h"

#include "comic.h"
#include "comiccontroller.h"
#include <QDataStream>
#include <QPointer>

PageController::PageController() {}

void PageController::service(HttpRequest& request, HttpResponse& response)
{
	HttpSession session=Static::sessionStore->getSession(request,response,false);

	QString path = QUrl::fromPercentEncoding(request.getPath()).toLatin1();
	QStringList pathElements = path.split('/');
	QString libraryName = pathElements.at(2);
	qulonglong comicId = pathElements.at(4).toULongLong();
	unsigned int page = pathElements.at(6).toUInt();

	Comic2 * comicFile = session.getCurrentComic();
	if(session.getCurrentComicId() != 0 && !QPointer<Comic2>(comicFile).isNull())
	{
		if(comicId == session.getCurrentComicId() && page < comicFile->numPages())
		{
			if(comicFile->pageIsLoaded(page))
			{
				response.setHeader("Content-Type", "image/jpeg");
				QByteArray pageData = comicFile->getRawPage(page);
				QDataStream data(pageData);
				char buffer[4096];
				while (!data.atEnd()) {
					int len = data.readRawData(buffer,4096);
					response.write(QByteArray(buffer,len));
				}
				response.write(pageData);
			}
			else
			{
				response.setStatus(404,"not found"); //TODO qué mensaje enviar
				response.write("404 not found",true);
			}
		}
		else
		{
			if(comicId != session.getCurrentComicId())
			{
				//delete comicFile;
				session.dismissCurrentComic();
			}
			response.setStatus(404,"not found"); //TODO qué mensaje enviar
			response.write("404 not found",true);
		}
	}
	else
	{
		response.setStatus(404,"not found");
		response.write("404 not found",true);
	}

	//response.write(t.toLatin1(),true);

}