#include "comicfullinfocontroller_v2.h"

#include <QUrl>

#include "db_helper.h"
#include "comic_db.h"

#include "yacreader_libraries.h"
#include "yacreader_server_data_helper.h"

#include <ctime>

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;
using namespace std;

ComicFullinfoController_v2::ComicFullinfoController_v2() { }

void ComicFullinfoController_v2::service(HttpRequest &request, HttpResponse &response)
{
    response.setHeader("Content-Type", "application/json");

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    int libraryId = pathElements.at(3).toInt();
    qulonglong comicId = pathElements.at(5).toULongLong();

    serviceContent(libraryId, comicId, response);
}

void ComicFullinfoController_v2::serviceContent(const int &libraryId, const qulonglong &comicId, HttpResponse &response)
{
    auto libraryUuid = DBHelper::getLibraries().getLibraryIdFromLegacyId(libraryId);

    ComicDB comic = DBHelper::getComicInfo(libraryId, comicId);

    if (!comic.info.existOnDb) {
        response.setStatus(404, "Not Found");
        response.write("", true);
        return;
    }

    QJsonObject json = YACReaderServerDataHelper::fullComicToJSON(libraryId, libraryUuid, comic);

    QJsonDocument output(json);

    response.write(output.toJson(QJsonDocument::Compact));

    response.setStatus(200, "OK");
    response.write("", true);
}
