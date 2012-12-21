#include "comiccontroller.h"

#include "library_window.h"

extern LibraryWindow * mw;

#include "template.h"
#include "../static.h"

#include "comic_db.h"
#include "comic.h"

Synchronizer::Synchronizer(Comic2 * c,QString p,QThread * f)
	:QThread(),comic(c),path(p),from(f)
{

}

void Synchronizer::run()
{
	connect(comic,SIGNAL(numPages(unsigned int)),this,SLOT(waitedSignal()));
	loaded = comic->load(path);
}

void Synchronizer::waitedSignal()
{
	if(loaded)
	{
		comic->moveToThread(from);
		quit();
	}
	else
	{
		exit(-1);
	}
}


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

	Comic2 * comicFile = new Comic2;
	//Synchronizer * synchronizer = new Synchronizer(comicFile,libraries.value(libraryName)+comic.path, this->thread());
	//comicFile->moveToThread(synchronizer);
	

	//synchronizer->start();
	//QApplication::instance()->exec();
	//synchronizer->wait(20000);

	if(comicFile->load(libraries.value(libraryName)+comic.path))
	{
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