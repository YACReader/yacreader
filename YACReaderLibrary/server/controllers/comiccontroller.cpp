#include "comiccontroller.h"

#include "db_helper.h"
#include "yacreader_libraries.h"

#include "template.h"
#include "../static.h"

#include "comic_db.h"
#include "comic.h"

#include "QsLog.h"

#include <typeinfo>

ComicController::ComicController() {}

void ComicController::service(HttpRequest& request, HttpResponse& response)
{
	HttpSession session=Static::sessionStore->getSession(request,response,false);

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
	QStringList pathElements = path.split('/');
    qulonglong libraryId = pathElements.at(2).toLongLong();
    QString libraryName = DBHelper::getLibraryName(libraryId);
	qulonglong comicId = pathElements.at(4).toULongLong();

    bool remoteComic = path.endsWith("remote");

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

	YACReaderLibraries libraries = DBHelper::getLibraries();
	
    ComicDB comic = DBHelper::getComicInfo(libraryId, comicId);

    if(!remoteComic)
        session.setDownloadedComic(comic.info.hash);

    Comic * comicFile = FactoryComic::newComic(libraries.getPath(libraryId)+comic.path);

	if(comicFile != NULL)
	{
		QThread * thread = NULL;

		thread = new QThread();

		comicFile->moveToThread(thread);

        connect(comicFile, SIGNAL(errorOpening()), thread, SLOT(quit()));
        connect(comicFile, SIGNAL(errorOpening(QString)), thread, SLOT(quit()));
        connect(comicFile, SIGNAL(imagesLoaded()), thread, SLOT(quit()));
		connect(thread, SIGNAL(started()), comicFile, SLOT(process()));
		connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

        comicFile->load(libraries.getPath(libraryId)+comic.path);

		if(thread != NULL)
			thread->start();

        if(remoteComic)
        {
            QLOG_TRACE() << "remote comic requested";
            session.setCurrentRemoteComic(comic.id, comicFile);

        }
        else
        {
            QLOG_TRACE() << "comic requested";
            session.setCurrentComic(comic.id, comicFile);
        }

        response.setHeader("Content-Type", "plain/text; charset=utf-8");
		//TODO this field is not used by the client!
		response.writeText(QString("library:%1\r\n").arg(libraryName));
        response.writeText(QString("libraryId:%1\r\n").arg(libraryId));
        if(remoteComic) //send previous and next comics id
        {
            QList<LibraryItem *> siblings = DBHelper::getFolderComicsFromLibrary(libraryId, comic.parentId, true);
            bool found = false;
            int i;
            for(i = 0; i < siblings.length(); i++)
            {
                if (siblings.at(i)->id == comic.id)
                {
                    found = true;
                    break;
                }
            }
            if(found)
            {
                if(i>0)
                    response.writeText(QString("previousComic:%1\r\n").arg(siblings.at(i-1)->id));
                if(i<siblings.length()-1)
                    response.writeText(QString("nextComic:%1\r\n").arg(siblings.at(i+1)->id));
            }
            else
            {
                //ERROR
            }
            qDeleteAll(siblings);
        }
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
