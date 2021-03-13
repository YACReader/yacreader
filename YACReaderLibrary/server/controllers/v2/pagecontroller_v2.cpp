#include "pagecontroller_v2.h"

#include "../static.h"

#include "comic.h"
#include "comiccontroller.h"
#include "yacreader_http_session.h"

#include <QDataStream>
#include <QPointer>

#include <QsLog.h>

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

PageControllerV2::PageControllerV2() { }

void PageControllerV2::service(HttpRequest &request, HttpResponse &response)
{
    QByteArray token = request.getHeader("x-request-id");
    YACReaderHttpSession *ySession = Static::yacreaderSessionStore->getYACReaderSessionHttpSession(token);

    if (ySession == nullptr) {
        response.setStatus(424, "no session for this comic");
        response.write("424 no session for this comic", true);
        return;
    }

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    bool remote = path.endsWith("remote");

    QStringList pathElements = path.split('/');
    qulonglong comicId = pathElements.at(5).toULongLong();
    unsigned int page = pathElements.at(7).toUInt();

    Comic *comicFile;
    qulonglong currentComicId;
    if (remote) {
        QLOG_TRACE() << "se recupera comic remoto para servir páginas";
        comicFile = ySession->getCurrentRemoteComic();
        currentComicId = ySession->getCurrentRemoteComicId();
    } else {
        QLOG_TRACE() << "se recupera comic para servir páginas";
        comicFile = ySession->getCurrentComic();
        currentComicId = ySession->getCurrentComicId();
    }

    if (comicFile->hasBeenAnErrorOpening()) {
        //delete comicFile;
        if (remote)
            ySession->dismissCurrentRemoteComic();
        else
            ySession->dismissCurrentComic();

        response.setStatus(404, "not found");
        response.write("404 not found", true);
        return;
    }

    if (currentComicId != 0 && !QPointer<Comic>(comicFile).isNull()) {
        if (comicFile->numPages() == 0) {
            response.setStatus(412, "opening file");
            response.write("412 opening file", true);
        } else {
            if (comicId == currentComicId && page < comicFile->numPages()) {
                if (comicFile->pageIsLoaded(page)) {
                    response.setHeader("Content-Type", "image/jpeg");
                    response.setHeader("Transfer-Encoding", "chunked");
                    QByteArray pageData = comicFile->getRawPage(page);
                    QDataStream data(pageData);
                    std::vector<char> buffer(100000);
                    while (!data.atEnd()) {
                        int len = data.readRawData(&buffer[0], buffer.size());
                        response.write(QByteArray(&buffer[0], len));
                    }
                    response.write(QByteArray(), true);
                } else {
                    response.setStatus(412, "loading page");
                    response.write("412 loading page", true);
                }
            } else {
                if (comicId != currentComicId) {
                    //delete comicFile;
                    if (remote)
                        ySession->dismissCurrentRemoteComic();
                    else
                        ySession->dismissCurrentComic();
                }
                response.setStatus(404, "not found");
                response.write("404 not found", true);
            }
        }
    } else {
        response.setStatus(404, "not found");
        response.write("404 not found", true);
    }
}
