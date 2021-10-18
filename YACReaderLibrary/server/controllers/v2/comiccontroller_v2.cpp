#include "comiccontroller_v2.h"

#include "db_helper.h"
#include "yacreader_libraries.h"
#include "yacreader_http_session.h"

#include "template.h"
#include "../static.h"

#include "comic_db.h"
#include "comic.h"

#include "qnaturalsorting.h"

#include "QsLog.h"

#include <typeinfo>

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

ComicControllerV2::ComicControllerV2() { }

void ComicControllerV2::service(HttpRequest &request, HttpResponse &response)
{

    QByteArray token = request.getHeader("x-request-id");
    YACReaderHttpSession *ySession = Static::yacreaderSessionStore->getYACReaderSessionHttpSession(token);

    if (ySession == nullptr) {
        response.setStatus(404, "not found");
        response.write("404 not found", true);
        return;
    }

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    qulonglong libraryId = pathElements.at(3).toLongLong();
    QString libraryName = DBHelper::getLibraryName(libraryId);
    qulonglong comicId = pathElements.at(5).toULongLong();

    bool remoteComic = path.endsWith("remote");

    // TODO
    // if(pathElements.size() == 6)
    //{
    //	QString action = pathElements.at(5);
    //	if(!action.isEmpty() && (action == "close"))
    //	{
    //		session.dismissCurrentComic();
    //		response.write("",true);
    //		return;
    //	}
    // }

    YACReaderLibraries libraries = DBHelper::getLibraries();

    ComicDB comic = DBHelper::getComicInfo(libraryId, comicId);

    Comic *comicFile = FactoryComic::newComic(libraries.getPath(libraryId) + comic.path);

    if (comicFile != nullptr) {
        QThread *thread = nullptr;

        thread = new QThread();

        comicFile->moveToThread(thread);

        connect(comicFile, QOverload<>::of(&Comic::errorOpening), thread, &QThread::quit);
        connect(comicFile, QOverload<QString>::of(&Comic::errorOpening), thread, &QThread::quit);
        connect(comicFile, &Comic::imagesLoaded, thread, &QThread::quit);
        connect(thread, &QThread::started, comicFile, &Comic::process);
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
        // TODO this field is not used by the client!
        response.write(QString("library:%1\r\n").arg(libraryName).toUtf8());
        response.write(QString("libraryId:%1\r\n").arg(libraryId).toUtf8());
        if (remoteComic) // send previous and next comics id
        {
            QList<LibraryItem *> siblings = DBHelper::getFolderComicsFromLibrary(libraryId, comic.parentId, false);

            std::sort(siblings.begin(), siblings.end(), LibraryItemSorter());

            bool found = false;
            int i;
            for (i = 0; i < siblings.length(); i++) {
                if (siblings.at(i)->id == comic.id) {
                    found = true;
                    break;
                }
            }
            if (found) {
                if (i > 0) {
                    ComicDB *previousComic = static_cast<ComicDB *>(siblings.at(i - 1));
                    response.write(QString("previousComic:%1\r\n").arg(previousComic->id).toUtf8());
                    response.write(QString("previousComicHash:%1\r\n").arg(previousComic->info.hash).toUtf8());
                }
                if (i < siblings.length() - 1) {
                    ComicDB *nextComic = static_cast<ComicDB *>(siblings.at(i + 1));
                    response.write(QString("nextComic:%1\r\n").arg(nextComic->id).toUtf8());
                    response.write(QString("nextComicHash:%1\r\n").arg(nextComic->info.hash).toUtf8());
                }
            } else {
                // ERROR
            }
            qDeleteAll(siblings);
        }
        response.write(comic.toTXT().toUtf8(), true);
    } else {
        // delete comicFile;
        response.setStatus(404, "not found");
        response.write("404 not found", true);
    }
    // response.write(t.toLatin1(),true);
}
