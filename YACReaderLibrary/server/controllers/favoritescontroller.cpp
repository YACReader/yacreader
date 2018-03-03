#include "favoritescontroller.h"

#include "db_helper.h"
#include "comic_db.h"

#include "yacreader_server_data_helper.h"

FavoritesController::FavoritesController() {}

void FavoritesController::service(HttpRequest &request, HttpResponse &response)
{
    response.setHeader("Content-Type", "text/plain; charset=utf-8");

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    int libraryId = pathElements.at(2).toInt();

    serviceContent(libraryId, response);

    response.write("",true);
}

void FavoritesController::serviceContent(const int library, HttpResponse &response)
{
    QList<ComicDB> tagComics = DBHelper::getFavorites(library);

    for(const ComicDB &comic : tagComics)
    {
        response.write(YACReaderServerDataHelper::comicToYSFormat(library, comic).toUtf8());
    }
}


