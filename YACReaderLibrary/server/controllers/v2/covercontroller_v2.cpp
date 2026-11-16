#include "covercontroller_v2.h"

#include "cover_request.h"
#include "db_helper.h" //get libraries
#include "yacreader_libraries.h"

using stefanfrings::HttpRequest;
using stefanfrings::HttpResponse;

CoverControllerV2::CoverControllerV2() { }

void CoverControllerV2::service(HttpRequest &request, HttpResponse &response)
{
    response.setHeader("Content-Type", "image/jpeg");

    YACReaderLibraries libraries = DBHelper::getLibraries();

    QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
    QStringList pathElements = path.split('/');
    QString libraryName = DBHelper::getLibraryName(pathElements.at(3).toInt());
    QStringList remainingPathElements = pathElements.mid(5);
    QString fileName = remainingPathElements.join('/');

    if (!serveCover(request, response, libraries.getPath(libraryName), fileName)) {
        response.setStatus(404, "not found");
        response.write("404 not found", true);
    }
}
