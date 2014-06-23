#include "pagecontroller.h"

#include "../static.h"

#include "comic.h"
#include "comiccontroller.h"
#include <QDataStream>
#include <QPointer>

#include "db_helper.h"

PageController::PageController() {}

void PageController::service(HttpRequest& request, HttpResponse& response)
{
	HttpSession session=Static::sessionStore->getSession(request,response,false);

	QString path = QUrl::fromPercentEncoding(request.getPath()).toLatin1();

	//QByteArray path2=request.getPath();
	//qDebug("PageController: request to -> %s ",path2.data());

	QStringList pathElements = path.split('/');
	QString libraryName = DBHelper::getLibraryName(pathElements.at(2).toInt());
	qulonglong comicId = pathElements.at(4).toULongLong();
	unsigned int page = pathElements.at(6).toUInt();

	//qDebug("lib name : %s",pathElements.at(2).data());

	Comic * comicFile = session.getCurrentComic();
	if(session.getCurrentComicId() != 0 && !QPointer<Comic>(comicFile).isNull())
	{
		if(comicId == session.getCurrentComicId() && page < comicFile->numPages())
		{
			if(comicFile->pageIsLoaded(page))
			{
				//qDebug("PageController: La página estaba cargada -> %s ",path.data());
				response.setHeader("Content-Type", "image/jpeg");
				response.setHeader("Transfer-Encoding","chunked");
				QByteArray pageData = comicFile->getRawPage(page);
				QDataStream data(pageData);
				char buffer[4096];
				while (!data.atEnd()) {
					int len = data.readRawData(buffer,4096);
					response.write(QByteArray(buffer,len));
				}
				//response.write(pageData,true);
			    response.write(QByteArray(),true);
			}
			else
			{
				//qDebug("PageController: La página NO estaba cargada 404 -> %s ",path.data());
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