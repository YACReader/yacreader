#include "readinglistscontroller.h"

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

}
