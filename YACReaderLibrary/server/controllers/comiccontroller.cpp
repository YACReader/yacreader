#include "comiccontroller.h"

#include "library_window.h"

extern LibraryWindow * mw;

#include "template.h"
#include "../static.h"

#include "comic_db.h"
#include "comic.h"

#include <typeinfo>

ComicController::ComicController() {}

void ComicController::service(HttpRequest& request, HttpResponse& response)
{
	HttpSession session=Static::sessionStore->getSession(request,response,false);

	QString path = QUrl::fromPercentEncoding(request.getPath()).toLatin1();
	QStringList pathElements = path.split('/');
	QString libraryName = pathElements.at(2);
	qulonglong comicId = pathElements.at(4).toULongLong();

	//TODO
	//if(pathElements.size() == 6)
	//{
	//	QString action = pathElements.at(5);
	//	if(!action.isEmpty() && (action == "close"))
	//	{
	//		session.dismissCurrentComic();
	//		response.write("",true);
	//		return;
	//	}
	//}

	QMap<QString,QString> libraries = mw->getLibraries();
	

	ComicDB comic = mw->getComicInfo(libraryName, comicId);

	Comic * comicFile = FactoryComic::newComic(libraries.value(libraryName)+comic.path);

	if(comicFile != NULL)
	{
		QThread * thread = NULL;
		if (typeid(*comicFile) != typeid(FileComic))
		{
			thread = new QThread();

			comicFile->moveToThread(thread);

			connect(thread, SIGNAL(started()), comicFile, SLOT(process()));
			connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

		}

		comicFile->load(libraries.value(libraryName)+comic.path);

		if(thread != NULL)
			thread->start();

		session.setCurrentComic(comic.id, comicFile);

		response.setHeader("Content-Type", "plain/text; charset=ISO-8859-1");

		response.writeText(QString("library:%1\r\n").arg(libraryName));
		response.writeText(comic.toTXT(),true);
	}
	else
	{
		//delete comicFile;
		response.setStatus(404,"not found");
		response.write("404 not found",true);
	}
	//response.write(t.toLatin1(),true);

}