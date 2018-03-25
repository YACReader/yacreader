#include "tagcontentcontroller_v2.h"

#include "db_helper.h"
#include "comic_db.h"

#include "yacreader_server_data_helper.h"

#include <QUrl>

TagContentControllerV2::TagContentControllerV2()
{

}

void TagContentControllerV2::service(HttpRequest &request, HttpResponse &response)
{
    response.setHeader("Content-Type", "text/plain; charset=utf-8");

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    int libraryId = pathElements.at(3).toInt();
    qulonglong tagId = pathElements.at(5).toULongLong();

    serviceContent(libraryId, tagId, response);

    response.write("",true);
}

void TagContentControllerV2::serviceContent(const int &library, const qulonglong &tagId, HttpResponse &response)
{
    QList<ComicDB> tagComics = DBHelper::getLabelComics(library, tagId);

    for(const ComicDB &comic : tagComics)
    {
        response.write(YACReaderServerDataHelper::comicToYSFormat(library, comic).toUtf8());
    }
}
