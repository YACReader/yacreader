#include "comiccontroller.h"

#include "library_window.h"

extern LibraryWindow * mw;

#include "template.h"
#include "../static.h"

#include "comic_db.h"
#include "comic.h"

ComicController::ComicController() {}

void ComicController::service(HttpRequest& request, HttpResponse& response)
{

	
	HttpSession session=Static::sessionStore->getSession(request,response);

	QStringList pathElements = ((QString)request.getPath()).split('/');
	QString libraryName = pathElements.at(2);
	qulonglong comicId = pathElements.at(4).toULongLong();

	QMap<QString,QString> libraries = mw->getLibraries();
	

	ComicDB comic = mw->getComicInfo(libraryName, comicId);

	Comic * comicFile = new Comic;
	if(comicFile->load(libraries.value(libraryName)+comic.path))
	{
		session.setCurrentComic(comic.id, comicFile);
		
		response.setHeader("Content-Type", "plain/text; charset=ISO-8859-1");
		response.writeText(QString("comicid:%1\n").arg(comic.id));
		response.writeText(QString("hash:%1\n").arg(comic.info.hash));
		response.writeText(QString("path:%1\n").arg(comic.path));
		response.writeText(QString("numpages:%1\n").arg(*comic.info.numPages));
		response.writeText(QString("library:%1\n").arg(libraryName),true);
	}
	else
	{
		delete comicFile;
		response.setStatus(404,"not found");
		response.write("404 not found",true);
	}


	

	//response.write(t.toLatin1(),true);

}