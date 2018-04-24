#include "readinglistcontentcontroller_v2.h"

#include "db_helper.h"
#include "comic_db.h"

#include "yacreader_server_data_helper.h"

ReadingListContentControllerV2::ReadingListContentControllerV2()
{

}

void ReadingListContentControllerV2::service(HttpRequest &request, HttpResponse &response)
{
    response.setHeader("Content-Type", "text/plain; charset=utf-8");

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    int libraryId = pathElements.at(3).toInt();
    qulonglong readingListId = pathElements.at(5).toULongLong();

    serviceContent(libraryId, readingListId, response);

    response.write("",true);
}

void ReadingListContentControllerV2::serviceContent(const int &library, const qulonglong &readingListId, HttpResponse &response)
{
    QList<ComicDB> comics = DBHelper::getReadingListFullContent(library, readingListId);

    QJsonArray items;

    for(const ComicDB &comic : comics)
    {
        items.append(YACReaderServerDataHelper::comicToJSON(library, comic));
    }

    QJsonDocument output(items);

    response.write(output.toJson());
}
