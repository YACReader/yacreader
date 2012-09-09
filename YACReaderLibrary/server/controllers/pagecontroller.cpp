#include "pagecontroller.h"

#include "../static.h"

#include "comic.h"

PageController::PageController() {}

void PageController::service(HttpRequest& request, HttpResponse& response)
{


	HttpSession session=Static::sessionStore->getSession(request,response);

	QStringList pathElements = ((QString)request.getPath()).split('/');
	QString libraryName = pathElements.at(2);
	qulonglong comicId = pathElements.at(4).toULongLong();
	quint16 page = pathElements.at(6).toUInt();

	Comic * comicFile = session.getCurrentComic();
	if((comicFile != 0) && comicId == session.getCurrentComicId())
	{
		response.setHeader("Content-Type", "image/page");
		QByteArray pageData = comicFile->getRawPage(page);
		response.write(pageData);
	}
	else
	{
		delete comicFile;
		response.setStatus(404,"not found");
		response.write("404 not found",true);
	}




	//response.write(t.toLatin1(),true);

}