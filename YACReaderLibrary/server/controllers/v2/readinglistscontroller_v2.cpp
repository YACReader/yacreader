#include "readinglistscontroller_v2.h"

#include "db_helper.h"
#include "reading_list.h"
#include "yacreader_server_data_helper.h"



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

    QJsonArray items;

    for(QList<ReadingList>::const_iterator itr = readingLists.constBegin();itr!=readingLists.constEnd();itr++)
    {
        items.append(YACReaderServerDataHelper::readingListToJSON(library, *itr));
    }

    QJsonDocument output(items);

    response.write(output.toJson(QJsonDocument::Compact));
}
