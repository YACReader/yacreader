#include "comiccontroller.h"

#include "db_helper.h"

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
	QString libraryName = DBHelper::getLibraryName(pathElements.at(2).toInt());
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

	//Aplicar a todos los controladores
	//TODO usar LibraryWindow para acceder a información de las bases de datos está mal, hay
	//que crear una clase que se encargue de estas cosas
	//¿Se está accediendo a la UI desde un hilo?

	QMap<QString,QString> libraries = DBHelper::getLibraries();
	

	ComicDB comic = DBHelper::getComicInfo(libraryName, comicId);

	session.setDownloadedComic(comic.info.hash);

	Comic * comicFile = FactoryComic::newComic(libraries.value(libraryName)+comic.path);

	if(comicFile != NULL)
	{
		QThread * thread = NULL;

		thread = new QThread();

		comicFile->moveToThread(thread);

		connect(thread, SIGNAL(started()), comicFile, SLOT(process()));
		connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

		comicFile->load(libraries.value(libraryName)+comic.path);

		if(thread != NULL)
			thread->start();

		session.setCurrentComic(comic.id, comicFile);

		response.setHeader("Content-Type", "plain/text; charset=ISO-8859-1");
		//TODO this field is not used by the client!
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