#include "comiccontroller.h"

#include "db_helper.h"
#include "yacreader_libraries.h"
#include "yacreader_http_session.h"

#include "template.h"
#include "../static.h"

#include "comic_db.h"
#include "comic.h"

#include "QsLog.h"

#include <typeinfo>

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;
using stefanfrings::HttpSession;

ComicController::ComicController() { }

void ComicController::service(HttpRequest &request, HttpResponse &response)
{
    HttpSession session = Static::sessionStore->getSession(request, response, false);
    YACReaderHttpSession *ySession = Static::yacreaderSessionStore->getYACReaderSessionHttpSession(session.getId());

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

    if (!remoteComic)
        ySession->setDownloadedComic(comic.info.hash);

    Comic *comicFile = FactoryComic::newComic(libraries.getPath(libraryId) + comic.path);

    if (comicFile != nullptr) {
        QThread *thread = nullptr;

        thread = new QThread();

        comicFile->moveToThread(thread);

        connect(comicFile, SIGNAL(errorOpening()), thread, SLOT(quit()));
        connect(comicFile, SIGNAL(errorOpening(QString)), thread, SLOT(quit()));
        connect(comicFile, &Comic::imagesLoaded, thread, &QThread::quit);
        connect(thread, SIGNAL(started()), comicFile, SLOT(process()));
        connect(thread, &QThread::finished, thread, &QObject::deleteLater);

        comicFile->load(libraries.getPath(libraryId) + comic.path);

        if (thread != nullptr)
            thread->start();

        if (remoteComic) {
            QLOG_TRACE() << "remote comic requested";
            ySession->setCurrentRemoteComic(comic.id, comicFile);

        } else {
            QLOG_TRACE() << "comic requested";
            ySession->setCurrentComic(comic.id, comicFile);
        }

        response.setHeader("Content-Type", "text/plain; charset=utf-8");
        //TODO this field is not used by the client!
        response.write(QString("library:%1\r\n").arg(libraryName).toUtf8());
        response.write(QString("libraryId:%1\r\n").arg(libraryId).toUtf8());
        if (remoteComic) //send previous and next comics id
        {
            QList<LibraryItem *> siblings = DBHelper::getFolderComicsFromLibrary(libraryId, comic.parentId, true);
            bool found = false;
            int i;
            for (i = 0; i < siblings.length(); i++) {
                if (siblings.at(i)->id == comic.id) {
                    found = true;
                    break;
                }
            }
            if (found) {
                if (i > 0)
                    response.write(QString("previousComic:%1\r\n").arg(siblings.at(i - 1)->id).toUtf8());
                if (i < siblings.length() - 1)
                    response.write(QString("nextComic:%1\r\n").arg(siblings.at(i + 1)->id).toUtf8());
            } else {
                //ERROR
            }
            qDeleteAll(siblings);
        }
        response.write(comic.toTXT().toUtf8(), true);
    } else {
        //delete comicFile;
        response.setStatus(404, "not found");
        response.write("404 not found", true);
    }
    //response.write(t.toLatin1(),true);
}
