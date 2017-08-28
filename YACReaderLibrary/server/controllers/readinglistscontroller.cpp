#include "readinglistscontroller.h"

#include "db_helper.h"
#include "reading_list.h"

ReadingListsController::ReadingListsController()
{

}

void ReadingListsController::service(HttpRequest &request, HttpResponse &response)
{
    response.setHeader("Content-Type", "text/plain; charset=utf-8");

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    int libraryId = pathElements.at(2).toInt();

    serviceContent(libraryId, response);

    response.write("",true);
}

void ReadingListsController::serviceContent(const int library, HttpResponse &response)
{
    QList<ReadingList> readingLists = DBHelper::getReadingLists(library);

    foreach(const ReadingList &item, readingLists)
    {
        response.write(QString("%1\t%2\t%3\r\n").arg(library).arg(item.getId()).arg(item.getName()).toUtf8());
    }
}
