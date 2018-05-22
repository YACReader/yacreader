/**
  @file
  @author Stefan Frings
*/

#include "requestmapper.h"
#include "static.h"
#include "staticfilecontroller.h"

#include "controllers/versioncontroller.h"

#include "controllers/v1/librariescontroller.h"
#include "controllers/v1/foldercontroller.h"
#include "controllers/v1/covercontroller.h"
#include "controllers/v1/comiccontroller.h"
#include "controllers/v1/folderinfocontroller.h"
#include "controllers/v1/pagecontroller.h"
#include "controllers/v1/updatecomiccontroller.h"
#include "controllers/v1/errorcontroller.h"
#include "controllers/v1/comicdownloadinfocontroller.h"
#include "controllers/v1/synccontroller.h"

#include "controllers/v2/librariescontroller_v2.h"
#include "controllers/v2/covercontroller_v2.h"
#include "controllers/v2/comiccontroller_v2.h"
#include "controllers/v2/folderinfocontroller_v2.h"
#include "controllers/v2/pagecontroller_v2.h"
#include "controllers/v2/updatecomiccontroller_v2.h"
#include "controllers/v2/errorcontroller_v2.h"
#include "controllers/v2/comicdownloadinfocontroller_v2.h"
#include "controllers/v2/synccontroller_v2.h"
#include "controllers/v2/foldercontentcontroller_v2.h"
#include "controllers/v2/tagscontroller_v2.h"
#include "controllers/v2/tagcontentcontroller_v2.h"
#include "controllers/v2/taginfocontroller_v2.h"
#include "controllers/v2/favoritescontroller_v2.h"
#include "controllers/v2/readingcomicscontroller_v2.h"
#include "controllers/v2/readinglistscontroller_v2.h"
#include "controllers/v2/readinglistcontentcontroller_v2.h"
#include "controllers/v2/readinglistinfocontroller_v2.h"
#include "controllers/v2/comicfullinfocontroller_v2.h"

#include "db_helper.h"
#include "yacreader_libraries.h"

#include "yacreader_http_session.h"

#include "QsLog.h"

QMutex RequestMapper::mutex;


RequestMapper::RequestMapper(QObject* parent)
	:HttpRequestHandler(parent) {}

void RequestMapper::loadSessionV1(HttpRequest & request, HttpResponse& response)
{
    QMutexLocker locker(&mutex);
    
    HttpSession session=Static::sessionStore->getSession(request,response);
    if(session.contains("ySession")) //session is already alive check if it is needed to update comics
    {
        YACReaderHttpSession *ySession = Static::yacreaderSessionStore->getYACReaderSessionHttpSession(session.getId());

        QString postData = QString::fromUtf8(request.getBody());

        if(postData.contains("currentPage"))
            return;

        if(postData.length()>0) {

            QList<QString> data = postData.split("\n");
            if(data.length() > 2) {
                ySession->setDeviceType(data.at(0).split(":").at(1));
                ySession->setDisplayType(data.at(1).split(":").at(1));
                QList<QString> comics = data.at(2).split(":").at(1).split("\t");
                ySession->clearComics();
                foreach(QString hash,comics) {
                    ySession->setComicOnDevice(hash);
                }
            }
            else
            {
                if(data.length()>1)
                {
                    ySession->setDeviceType(data.at(0).split(":").at(1));
                    ySession->setDisplayType(data.at(1).split(":").at(1));
                }
            }
        }
    }
    else
    {
        YACReaderHttpSession *ySession = new YACReaderHttpSession(this);

        Static::yacreaderSessionStore->addYACReaderHttpSession(session.getId(), ySession);

        session.set("ySession","ok");

        QString postData = QString::fromUtf8(request.getBody());
        //response.writeText(postData);

        QList<QString> data = postData.split("\n");

        if(data.length() > 2)
        {
            ySession->setDeviceType(data.at(0).split(":").at(1));
            ySession->setDisplayType(data.at(1).split(":").at(1));
            QList<QString> comics = data.at(2).split(":").at(1).split("\t");
            foreach(QString hash,comics)
            {
                ySession->setComicOnDevice(hash);
            }
        }
        else //values by default, only for debug purposes.
        {
            ySession->setDeviceType("ipad");
            ySession->setDisplayType("@2x");
        }

    }
}

void RequestMapper::loadSessionV2(HttpRequest & request, HttpResponse& response)
{
    QMutexLocker locker(&mutex);
    
    QByteArray token = request.getHeader("x-request-id");
    
    if (token.isEmpty()) {
        return;
    }
    
    YACReaderHttpSession *yRecoveredSession = Static::yacreaderSessionStore->getYACReaderSessionHttpSession(token);
    
    if(yRecoveredSession == nullptr) //session is already alive check if it is needed to update comics
    {
        YACReaderHttpSession *ySession = new YACReaderHttpSession(this);
        
        Static::yacreaderSessionStore->addYACReaderHttpSession(token, ySession);
    }
}

void RequestMapper::service(HttpRequest& request, HttpResponse& response) {
    QByteArray path=request.getPath();
    
    QLOG_TRACE() << "RequestMapper: path=" << path.data();
    QLOG_TRACE() << "X-Request-Id: " << request.getHeader("x-request-id");

    if (path.startsWith("/v2"))
    {
        serviceV2(request, response);
    }
    else
    {
        serviceV1(request, response);
    }
}

void RequestMapper::serviceV1(HttpRequest& request, HttpResponse& response)
{
    QByteArray path=request.getPath();

    QRegExp folder("/library/.+/folder/[0-9]+/?");//get comic content
    QRegExp folderInfo("/library/.+/folder/[0-9]+/info/?"); //get folder info
    QRegExp comicDownloadInfo("/library/.+/comic/[0-9]+/?"); //get comic info (basic/download info)
    QRegExp comicFullInfo("/library/.+/comic/[0-9]+/info/?"); //get comic info (full info)
    QRegExp comicOpen("/library/.+/comic/[0-9]+/remote/?"); //the server will open for reading the comic
    QRegExp comicUpdate("/library/.+/comic/[0-9]+/update/?"); //get comic info
    QRegExp comicClose("/library/.+/comic/[0-9]+/close/?"); //the server will close the comic and free memory
    QRegExp cover("/library/.+/cover/[0-9a-f]+.jpg"); //get comic cover (navigation)
    QRegExp comicPage("/library/.+/comic/[0-9]+/page/[0-9]+/?"); //get comic page
    QRegExp comicPageRemote("/library/.+/comic/[0-9]+/page/[0-9]+/remote?"); //get comic page (remote reading)

    QRegExp sync("/sync");

    QRegExp library("/library/([0-9]+)/.+"); //permite verificar que la biblioteca solicitada existe

    path = QUrl::fromPercentEncoding(path).toUtf8();

    if(!sync.exactMatch(path)) //no session is needed for syncback info, until security will be added
        loadSessionV1(request, response);

    //primera petición, se ha hecho un post, se sirven las bibliotecas si la seguridad mediante login no está habilitada
    if(path == "/")  //Don't send data to the server using '/' !!!!
    {
        LibrariesController().service(request, response);
    }
    else
    {
        if(sync.exactMatch(path))
            SyncController().service(request, response);
        else
        {
            //se comprueba que la sesión sea la correcta con el fin de evitar accesos no autorizados
            HttpSession session=Static::sessionStore->getSession(request,response,false);
            if(!session.isNull() && session.contains("ySession"))
            {
                if(library.indexIn(path)!=-1 && DBHelper::getLibraries().contains(library.cap(1).toInt()) )
                {
                    //listar el contenido del folder
                    if(folder.exactMatch(path))
                    {
                        FolderController().service(request, response);
                    }
                    else if (folderInfo.exactMatch(path))
                    {
                        FolderInfoController().service(request, response);
                    }
                    else if(cover.exactMatch(path))
                    {
                        CoverController().service(request, response);
                    }
                    else if(comicDownloadInfo.exactMatch(path))
                    {
                        ComicDownloadInfoController().service(request, response);
                    }
                    else if(comicFullInfo.exactMatch(path) || comicOpen.exactMatch(path))//start download or start remote reading
                    {
                        ComicController().service(request, response);
                    }
                    else if(comicPage.exactMatch(path) || comicPageRemote.exactMatch(path))
                    {
                        PageController().service(request,response);
                    }
                    else if(comicUpdate.exactMatch(path))
                    {
                        UpdateComicController().service(request, response);
                    }
                }
                else
                {
                    //response.writeText(library.cap(1));
                    Static::staticFileController->service(request, response);
                }
            }
            else //acceso no autorizado, redirección
            {
                ErrorController(300).service(request,response);
            }
        }
    }
}

void RequestMapper::serviceV2(HttpRequest& request, HttpResponse& response)
{
    QByteArray path=request.getPath();

    QRegExp folderInfo("/v2/library/.+/folder/[0-9]+/info/?"); //get folder info
    QRegExp comicDownloadInfo("/v2/library/.+/comic/[0-9]+/?"); //get comic info (basic/download info)
    QRegExp comicOpenForDownloading("/v2/library/.+/comic/[0-9]+/info/?"); //get comic info (full info + opening)
    QRegExp comicOpenForRemoteReading("/v2/library/.+/comic/[0-9]+/remote/?"); //the server will open for reading the comic
    QRegExp comicFullInfo("/v2/library/.+/comic/[0-9]+/fullinfo/?"); //get comic info
    QRegExp comicUpdate("/v2/library/.+/comic/[0-9]+/update/?"); //get comic info
    QRegExp comicClose("/v2/library/.+/comic/[0-9]+/close/?"); //the server will close the comic and free memory
    QRegExp cover("/v2/library/.+/cover/[0-9a-f]+.jpg"); //get comic cover (navigation)
    QRegExp comicPage("/v2/library/.+/comic/[0-9]+/page/[0-9]+/?"); //get comic page
    QRegExp comicPageRemote("/v2/library/.+/comic/[0-9]+/page/[0-9]+/remote?"); //get comic page (remote reading)
    QRegExp serverVersion("/v2/version/?");
    QRegExp folderContent("/v2/library/.+/folder/[0-9]+/content/?");
    QRegExp favs("/v2/library/.+/favs/?");
    QRegExp reading("/v2/library/.+/reading/?");
    QRegExp tags("/v2/library/.+/tags/?");
    QRegExp tagContent("/v2/library/.+/tag/[0-9]+/content/?");
    QRegExp tagInfo("/v2/library/.+/tag/[0-9]+/info/?");
    QRegExp readingLists("/v2/library/.+/reading_lists/?");
    QRegExp readingListContent("/v2/library/.+/reading_list/[0-9]+/content/?");
    QRegExp readingListInfo("/v2/library/.+/reading_list/[0-9]+/info/?");

    QRegExp sync("/v2/sync");

    QRegExp library("/v2/library/([0-9]+)/.+"); //permite verificar que la biblioteca solicitada existe

    path = QUrl::fromPercentEncoding(path).toUtf8();

    if(!sync.exactMatch(path)) //no session is needed for syncback info, until security will be added
        loadSessionV2(request, response);

    //primera petición, se ha hecho un post, se sirven las bibliotecas si la seguridad mediante login no está habilitada
    if(path == "/v2/libraries")  //Don't send data to the server using '/' !!!!
    {
        LibrariesControllerV2().service(request, response);
    }
    else
    {
        if(serverVersion.exactMatch(path))
        {
            VersionController().service(request, response);
        }
        else if(sync.exactMatch(path))
        {
            SyncControllerV2().service(request, response);
        }
        else
        {
                if(library.indexIn(path)!=-1 && DBHelper::getLibraries().contains(library.cap(1).toInt()) )
                {
                    if (folderInfo.exactMatch(path))
                    {
                        FolderInfoControllerV2().service(request, response);
                    }
                    else if(cover.exactMatch(path))
                    {
                        CoverControllerV2().service(request, response);
                    }
                    else if(comicDownloadInfo.exactMatch(path))
                    {
                        ComicDownloadInfoControllerV2().service(request, response);
                    }
                    else if(comicOpenForDownloading.exactMatch(path) || comicOpenForRemoteReading.exactMatch(path))//start download or start remote reading
                    {
                        ComicControllerV2().service(request, response);
                    } else if(comicFullInfo.exactMatch(path)) {
                        ComicFullinfoController_v2().service(request, response);
                    }
                    else if(comicPage.exactMatch(path) || comicPageRemote.exactMatch(path))
                    {
                        PageControllerV2().service(request,response);
                    }
                    else if(comicUpdate.exactMatch(path))
                    {
                        UpdateComicControllerV2().service(request, response);
                    }
                    else if(folderContent.exactMatch(path))
                    {
                        FolderContentControllerV2().service(request, response);
                    }
                    else if(tags.exactMatch(path))
                    {
                        TagsControllerV2().service(request, response);
                    }
                    else if(tagContent.exactMatch(path))
                    {
                        TagContentControllerV2().service(request, response);
                    }
                    else if(favs.exactMatch(path))
                    {
                        FavoritesControllerV2().service(request, response);
                    }
                    else if(reading.exactMatch(path))
                    {
                        ReadingComicsControllerV2().service(request, response);
                    }
                    else if(readingLists.exactMatch(path))
                    {
                        ReadingListsControllerV2().service(request, response);
                    }
                    else if(readingListContent.exactMatch(path))
                    {
                        ReadingListContentControllerV2().service(request, response);
                    }
                    else if(readingListInfo.exactMatch(path))
                    {
                        ReadingListInfoControllerV2().service(request, response);
                    }
                    else if(tagInfo.exactMatch(path))
                    {
                        TagInfoControllerV2().service(request, response);
                    }
                }
                else
                {
                    //response.writeText(library.cap(1));
                    Static::staticFileController->service(request, response);
                }
        }
    }
}
