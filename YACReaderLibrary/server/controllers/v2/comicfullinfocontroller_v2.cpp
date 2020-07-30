#include "comicfullinfocontroller_v2.h"

#include <QUrl>

#include "db_helper.h"
#include "comic_db.h"
#include "folder.h"

#include "yacreader_server_data_helper.h"

#include "qnaturalsorting.h"

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

    response.setStatus(200, "OK");
    response.write("", true);
}

void ComicFullinfoController_v2::serviceContent(const int &libraryId, const qulonglong &comicId, HttpResponse &response)
{
    ComicDB comic = DBHelper::getComicInfo(libraryId, comicId);

    QJsonObject json = YACReaderServerDataHelper::fullComicToJSON(libraryId, comic);

    QJsonDocument output(json);

    response.write(output.toJson(QJsonDocument::Compact));
}
