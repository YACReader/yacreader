#include "readinglistscontroller_v2.h"

#include "db_helper.h"
#include "reading_list.h"

ReadingListsControllerV2::ReadingListsControllerV2()
{

}

void ReadingListsControllerV2::service(HttpRequest &request, HttpResponse &response)
{
    response.setHeader("Content-Type", "text/plain; charset=utf-8");

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    int libraryId = pathElements.at(3).toInt();

    serviceContent(libraryId, response);

    response.write("",true);
}

void ReadingListsControllerV2::serviceContent(const int library, HttpResponse &response)
{
    QList<ReadingList> readingLists = DBHelper::getReadingLists(library);

    foreach(const ReadingList &item, readingLists)
    {
        response.write(QString("%1\t%2\t%3\r\n").arg(library).arg(item.getId()).arg(item.getName()).toUtf8());
    }
}
