#include "favoritescontroller_v2.h"

#include "db_helper.h"
#include "comic_db.h"

#include "yacreader_server_data_helper.h"

FavoritesControllerV2::FavoritesControllerV2() {}

void FavoritesControllerV2::service(HttpRequest &request, HttpResponse &response)
{
    response.setHeader("Content-Type", "text/plain; charset=utf-8");

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    int libraryId = pathElements.at(3).toInt();

    serviceContent(libraryId, response);

    response.write("", true);
}

void FavoritesControllerV2::serviceContent(const int library, HttpResponse &response)
{
    QList<ComicDB> comics = DBHelper::getFavorites(library);

    QJsonArray items;

    for (const ComicDB &comic : comics) {
        items.append(YACReaderServerDataHelper::comicToJSON(library, comic));
    }

    QJsonDocument output(items);

    response.write(output.toJson(QJsonDocument::Compact));
}
