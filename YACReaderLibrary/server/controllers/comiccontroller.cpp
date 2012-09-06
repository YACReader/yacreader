#include "comiccontroller.h"
#include "library_window.h"

extern LibraryWindow * mw;

#include "template.h"
#include "../static.h"

#include "comic_db.h"

ComicController::ComicController() {}

void ComicController::service(HttpRequest& request, HttpResponse& response)
{
	response.setHeader("Content-Type", "plain/text; charset=ISO-8859-1");
	
	QStringList pathElements = ((QString)request.getPath()).split('/');
	QString libraryName = pathElements.at(2);
	qulonglong comicId = pathElements.at(4).toULongLong();

	ComicDB comic = mw->getComicInfo(libraryName, comicId);

	response.writeText(QString("comicid:%1\n").arg(comic.id));
	response.writeText(QString("hash:%1\n").arg(comic.info.hash));
	response.writeText(QString("path:%1\n").arg(comic.path));
	response.writeText(QString("numpages:%1\n").arg(*comic.info.numPages));
	response.writeText(QString("library:%1\n").arg(libraryName),true);
	
	//response.write(t.toLatin1(),true);

}