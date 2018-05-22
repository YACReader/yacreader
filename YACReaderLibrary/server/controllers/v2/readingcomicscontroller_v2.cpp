#include "readingcomicscontroller_v2.h"

#include "db_helper.h"
#include "comic_db.h"

#include "yacreader_server_data_helper.h"

ReadingComicsControllerV2::ReadingComicsControllerV2()
{

}

void ReadingComicsControllerV2::service(HttpRequest &request, HttpResponse &response)
{
    response.setHeader("Content-Type", "application/json");

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    int libraryId = pathElements.at(3).toInt();

    serviceContent(libraryId, response);

    response.setStatus(200,"OK");
    response.write("",true);
}

void ReadingComicsControllerV2::serviceContent(const int &library, HttpResponse &response)
{
    QList<ComicDB> readingComics = DBHelper::getReading(library);

    QJsonArray comics;

    for(const ComicDB &comic : readingComics)
    {
        comics.append(YACReaderServerDataHelper::comicToJSON(library, comic));
    }

    QJsonDocument output(comics);

    response.write(output.toJson(QJsonDocument::Compact));
}
