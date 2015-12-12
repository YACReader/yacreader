#include "comicdownloadinfocontroller.h"

#include "db_helper.h"
#include "yacreader_libraries.h"

#include "comic_db.h"

ComicDownloadInfoController::ComicDownloadInfoController() {}


void ComicDownloadInfoController::service(HttpRequest& request, HttpResponse& response)
{
    response.setHeader("Content-Type", "plain/text; charset=utf-8");

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');

    qulonglong libraryId = pathElements.at(2).toLongLong();
    qulonglong comicId = pathElements.at(4).toULongLong();

    ComicDB comic = DBHelper::getComicInfo(libraryId, comicId);

    //TODO: check if the comic wasn't found;
    response.writeText(QString("fileName:%1\r\n").arg(comic.getFileName()));
    response.writeText(QString("fileSize:%1\r\n").arg(comic.getFileSize()),true);
}
