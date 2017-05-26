#include "readinglistcontentcontroller.h"

#include "db_helper.h"
#include "comic_db.h"

#include "yacreader_server_data_helper.h"

ReadingListContentController::ReadingListContentController()
{

}

void ReadingListContentController::service(HttpRequest &request, HttpResponse &response)
{
    response.setHeader("Content-Type", "text/plain; charset=utf-8");

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    int libraryId = pathElements.at(2).toInt();
    qulonglong readingListId = pathElements.at(4).toULongLong();

    serviceContent(libraryId, readingListId, response);

    response.write("",true);
}

void ReadingListContentController::serviceContent(const int &library, const qulonglong &readingListId, HttpResponse &response)
{
    QList<ComicDB> comics = DBHelper::getReadingListFullContent(library, readingListId);

    for(const ComicDB &comic : comics)
    {
        response.write(YACReaderServerDataHelper::comicToYSFormat(library, comic).toUtf8());
    }
}
