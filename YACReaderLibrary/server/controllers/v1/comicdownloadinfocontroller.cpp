#include "comicdownloadinfocontroller.h"

#include "db_helper.h"
#include "yacreader_libraries.h"

#include "comic_db.h"

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

ComicDownloadInfoController::ComicDownloadInfoController() { }

void ComicDownloadInfoController::service(HttpRequest &request, HttpResponse &response)
{
    response.setHeader("Content-Type", "text/plain; charset=utf-8");

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');

    qulonglong libraryId = pathElements.at(2).toLongLong();
    qulonglong comicId = pathElements.at(4).toULongLong();

    ComicDB comic = DBHelper::getComicInfo(libraryId, comicId);

    //TODO: check if the comic wasn't found;
    response.write(QString("fileName:%1\r\n").arg(comic.getFileName()).toUtf8());
    response.write(QString("fileSize:%1\r\n").arg(comic.getFileSize()).toUtf8(), true);
}
